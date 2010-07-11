/*
    Copyright (c) 2010 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "eventdatacontainer.h"

#include <KSystemTimeZones>

#include <KCal/Calendar>
#include <KCal/Event>
#include <KCal/Todo>
#include <KCal/Journal>

#include "akonadi/calendar.h"
#include "akonadi/calendarmodel.h"

using namespace Akonadi;

EventDataContainer::EventDataContainer(Akonadi::Calendar* calendar, const QString& name, const KDateTime& start, const KDateTime& end, QObject* parent)
                  : Plasma::DataContainer(parent),
                    m_calendar(calendar),
                    m_name(name),
                    m_startDate(start),
                    m_endDate(end)
{
    // name under which this dataEngine source appears
    setObjectName(name);

    // Connect directly to the calendar for now
    connect(calendar, SIGNAL(calendarChanged()), this, SLOT(updateData()));

    // create the initial data
    updateData();
}

void EventDataContainer::updateData()
{
    removeAllData();
    updateEventData();
    updateTodoData();
    updateJournalData();
    checkForUpdate();
}

void EventDataContainer::updateEventData()
{
    Akonadi::Item::List events = m_calendar->events(m_startDate.date(), m_endDate.date(), m_calendar->timeSpec());

    foreach (const Akonadi::Item &item, events) {
        Q_ASSERT(item.hasPayload<KCal::Event::Ptr>());
        const KCal::Event::Ptr event = item.payload<KCal::Event::Ptr>();

        Plasma::DataEngine::Data eventData;

        populateIncidenceData(event, eventData);

        // Event specific fields
        eventData["EventMultiDay"] = event->allDay();
        eventData["EventHasEndDate"] = event->hasEndDate();
        if (event->transparency() == KCal::Event::Opaque) {
            eventData["EventTransparency"] = "Opaque";
        } else if (event->transparency() == KCal::Event::Transparent) {
            eventData["EventTransparency"] = "Transparent";
        } else {
            eventData["EventTransparency"] = "Unknown";
        }

        setData(event->uid(), eventData);
    }
}

void EventDataContainer::updateTodoData()
{
    QDate todoDate = m_startDate.date();
    while(todoDate <= m_endDate.date()) {
        Akonadi::Item::List todos = m_calendar->todos(todoDate);

        foreach (const Akonadi::Item &item, todos) {
            Q_ASSERT(item.hasPayload<KCal::Todo::Ptr>());
            const KCal::Todo::Ptr todo = item.payload<KCal::Todo::Ptr>();

            Plasma::DataEngine::Data todoData;

            populateIncidenceData(todo, todoData);

            QVariant var;
            // Todo specific fields
            todoData["TodoHasStartDate"] = todo->hasStartDate();
            todoData["TodoIsOpenEnded"] = todo->isOpenEnded();
            todoData["TodoHasDueDate"] = todo->hasDueDate();
            var.setValue(todo->dtDue());
            todoData["TodoDueDate"] = var;
            todoData["TodoIsCompleted"] = todo->isCompleted();
            todoData["TodoIsInProgress"] = todo->isInProgress(false); // ???
            todoData["TodoIsNotStarted"] = todo->isNotStarted(false); // ???
            todoData["TodoPercentComplete"] = todo->percentComplete();
            todoData["TodoHasCompletedDate"] = todo->hasCompletedDate();
            var.setValue(todo->completed());
            todoData["TodoCompletedDate"] = var;

            setData(todo->uid(), todoData);
        }

        todoDate = todoDate.addDays(1);
    }
}

void EventDataContainer::updateJournalData()
{
    QDate journalDate = m_startDate.date();
    while(journalDate <= m_endDate.date()) {
        Akonadi::Item::List journals = m_calendar->journals(journalDate);

        foreach (const Akonadi::Item &item, journals) {
            Q_ASSERT(item.hasPayload<KCal::Journal::Ptr>());
            const KCal::Journal::Ptr journal = item.payload<KCal::Journal::Ptr>();

            Plasma::DataEngine::Data journalData;

            populateIncidenceData(journal, journalData);

            // No Journal specific fields

            setData(journal->uid(), journalData);
        }

        journalDate = journalDate.addDays(1);
    }
}

void EventDataContainer::populateIncidenceData(KCal::Incidence::Ptr incidence, Plasma::DataEngine::Data &incidenceData)
{
    QVariant var;
    incidenceData["UID"] = incidence->uid();
    incidenceData["Type"] = incidence->type();
    incidenceData["Summary"] = incidence->summary();
    incidenceData["Comments"] = incidence->comments();
    incidenceData["Location"] = incidence->location();
    incidenceData["OrganizerName"] = incidence->organizer().name();
    incidenceData["OrganizerEmail"] = incidence->organizer().email();
    incidenceData["Priority"] = incidence->priority();
    var.setValue(incidence->dtStart());
    incidenceData["StartDate"] = var;
    var.setValue(incidence->dtEnd());
    incidenceData["EndDate"] = var;
    // Build the Occurance Index, this lists all occurences of the Incidence in the required range
    // Single occurance events just repeat the standard start/end dates
    // Recurring Events use each recurrence start/end date
    // The OccurenceUid is redundant, but it makes it easy for clients to just take() the data structure intact as a separate index
    QList<QVariant> occurences;
    // Build the recurrence list of start dates only for recurring incidences only
    QList<QVariant> recurrences;
    if (incidence->recurs()) {
        KCal::DateTimeList recurList = incidence->recurrence()->timesInInterval(m_startDate, m_endDate);
        foreach(const KDateTime &recurDateTime, recurList) {
            var.setValue(recurDateTime);
            recurrences.append(var);
            Plasma::DataEngine::Data occurence;
            occurence.insert("OccurrenceUid", incidence->uid());
            occurence.insert("OccurrenceStartDate", var);
            var.setValue(incidence->endDateForStart(recurDateTime));
            occurence.insert("OccurrenceEndDate", var);
            occurences.append(QVariant(occurence));
        }
    } else {
        Plasma::DataEngine::Data occurence;
        occurence.insert("OccurrenceUid", incidence->uid());
        var.setValue(incidence->dtStart());
        occurence.insert("OccurrenceStartDate", var);
        var.setValue(incidence->dtEnd());
        occurence.insert("OccurrenceEndDate", var);
        occurences.append(QVariant(occurence));
    }
    incidenceData["RecurrenceDates"] = QVariant(recurrences);
    incidenceData["Occurrences"] = QVariant(occurences);
    if (incidence->status() == KCal::Incidence::StatusNone) {
        incidenceData["Status"] = "None";
    } else if (incidence->status() == KCal::Incidence::StatusTentative) {
        incidenceData["Status"] = "Tentative";
    } else if (incidence->status() == KCal::Incidence::StatusConfirmed) {
        incidenceData["Status"] = "Confirmed";
    } else if (incidence->status() == KCal::Incidence::StatusDraft) {
        incidenceData["Status"] = "Draft";
    } else if (incidence->status() == KCal::Incidence::StatusFinal) {
        incidenceData["Status"] = "Final";
    } else if (incidence->status() == KCal::Incidence::StatusCompleted) {
        incidenceData["Status"] = "Completed";
    } else if (incidence->status() == KCal::Incidence::StatusInProcess) {
        incidenceData["Status"] = "InProcess";
    } else if (incidence->status() == KCal::Incidence::StatusCanceled) {
        incidenceData["Status"] = "Cancelled";
    } else if (incidence->status() == KCal::Incidence::StatusNeedsAction) {
        incidenceData["Status"] = "NeedsAction";
    } else if (incidence->status() == KCal::Incidence::StatusX) {
        incidenceData["Status"] = "NonStandard";
    } else {
        incidenceData["Status"] = "Unknown";
    }
    incidenceData["StatusName"] = incidence->statusStr();

    if (incidence->secrecy() == KCal::Incidence::SecrecyPublic) {
        incidenceData["Secrecy"] = "Public";
    } else if (incidence->secrecy() == KCal::Incidence::SecrecyPrivate) {
        incidenceData["Secrecy"] = "Private";
    } else if (incidence->secrecy() == KCal::Incidence::SecrecyConfidential) {
        incidenceData["Secrecy"] = "Confidential";
    } else {
        incidenceData["Secrecy"] = "Unknown";
    }
    incidenceData["SecrecyName"] = incidence->secrecyStr();
    incidenceData["Recurs"] = incidence->recurs();
    incidenceData["AllDay"] = incidence->allDay();
    incidenceData["Categories"] = incidence->categories();
    incidenceData["Resources"] = incidence->resources();
    incidenceData["DurationDays"] = incidence->duration().asDays();
    incidenceData["DurationSeconds"] = incidence->duration().asSeconds();

    //TODO Attendees
    //TODO Attachments
    //TODO Relations
    //TODO Alarms
    //TODO Custom Properties
    //TODO Lat/Lon
    //TODO Collection/Source
}

#include "eventdatacontainer.moc"
