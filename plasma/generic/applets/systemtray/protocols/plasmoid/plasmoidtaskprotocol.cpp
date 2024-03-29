/***************************************************************************
 *   plasmoidprotocol.cpp                                                  *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "plasmoidtask.h"
#include "plasmoidtaskprotocol.h"

#include <QDebug>

#include <Plasma/Applet>

namespace SystemTray
{

PlasmoidProtocol::PlasmoidProtocol(QObject *parent)
    : Protocol(parent)
{
}


PlasmoidProtocol::~PlasmoidProtocol()
{
}


void PlasmoidProtocol::init()
{
}

void PlasmoidProtocol::forwardConstraintsEvent(Plasma::Types::Constraints constraints, Plasma::Applet *host)
{
    if (m_tasks.contains(host)) {
        QHash<QString, PlasmoidTask*> tasksForHost = m_tasks.value(host);
        foreach (PlasmoidTask *task, tasksForHost) {
            task->forwardConstraintsEvent(constraints);
        }
    }
}

void PlasmoidProtocol::loadFromConfig(Plasma::Applet *parent)
{
    QHash<QString, PlasmoidTask*> existingTasks = m_tasks.value(parent);
    QSet<QString> seenNames;

    KConfigGroup appletGroup = parent->config();
    appletGroup = KConfigGroup(&appletGroup, "Applets");

    foreach (const QString &groupName, appletGroup.groupList()) {
        const KConfigGroup childGroup(&appletGroup, groupName);
        const QString appletName = childGroup.readEntry("plugin", QString());

        existingTasks.remove(appletName);
        addApplet(appletName, groupName.toInt(), parent);
    }

    QHashIterator<QString, PlasmoidTask*> it(existingTasks);
    while (it.hasNext()) {
        it.next();
        /*
        Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(it.value()->widget(parent, true));
        if (applet) {
            applet->destroy();
        }
        */
    }
}

void PlasmoidProtocol::addApplet(const QString appletName, const int id, Plasma::Applet *parent)
{
    PlasmoidTask *task = m_tasks.value(parent).value(appletName);
    if (task) {
        // the host already has one of these applets ... but let's make sure that the id is the same
        // if it isn't, we have a duplicate in the config file and it should be removed
        if (task->id() != id) {
            KConfigGroup appletGroup = parent->config();
            appletGroup = KConfigGroup(&appletGroup, "Applets");
            appletGroup = KConfigGroup(&appletGroup, QString::number(id));
            appletGroup.deleteGroup();
        }

        return;
    }

    qDebug() << "Registering task with the manager" << appletName;
    task = new PlasmoidTask(appletName, id, this, parent);

    if (!task->isValid()) {
        // we failed to load our applet *sob*
        delete task;
        return;
    }


    m_tasks[parent].insert(appletName, task);
    connect(task, SIGNAL(taskDeleted(Plasma::Applet*,QString)), this, SLOT(cleanupTask(Plasma::Applet*,QString)));
    emit taskCreated(task);
}

void PlasmoidProtocol::removeApplet(const QString appletName, Plasma::Applet *parent)
{
    if (!m_tasks.contains(parent) || !m_tasks.value(parent).contains(appletName)) {
        return;
    }

//     Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(m_tasks.value(parent).value(appletName)->widget(parent, true));
//
//     if (applet) {
//         applet->destroy();
//     }
}

void PlasmoidProtocol::cleanupTask(Plasma::Applet *host, const QString &taskId)
{
    qDebug() << "task with taskId" << taskId << "removed";
    if (m_tasks.contains(host)) {
        m_tasks[host].remove(taskId);
        if (m_tasks.value(host).isEmpty()) {
            m_tasks.remove(host);
        }
    }
}

QStringList PlasmoidProtocol::applets(Plasma::Applet *host) const
{
    QStringList list;
    if (!m_tasks.contains(host)) {
        return list;
    }

    QHashIterator<QString, PlasmoidTask *> i(m_tasks.value(host));

    while (i.hasNext()) {
        i.next();
        list << i.key();
    }

    return list;
}

}

#include "plasmoidtaskprotocol.moc"
