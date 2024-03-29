/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "desktopcorona.h"
#include "plasma-shell-desktop.h"
#include "activitymanager/kidenticongenerator.h"

#include <QPixmap>
#include <QString>
#include <QSize>
#include <QFile>

#include <KConfig>
#include <KIcon>
#include <KStandardDirs>
#include <KWindowSystem>

#include <Plasma/Containment>
#include <Plasma/Context>
#include <Plasma/Corona>

#include <KActivities/Controller>
#include <KActivities/Consumer>

#include "plasmaapp.h"

#include "activity.h"

Activity::Activity(const QString &id, QObject *parent)
    : QObject(parent),
      m_id(id),
      m_plugin("default"),
      m_info(new KActivities::Info(id, this)),
      m_activityConsumer(new KActivities::Consumer(this)),
      m_current(false)
{
    m_name = m_info->name();
    m_icon = m_info->icon();

    connect(m_info, SIGNAL(infoChanged()), this, SLOT(activityChanged()));
    connect(m_info, SIGNAL(stateChanged(KActivities::Info::State)), this, SLOT(activityStateChanged(KActivities::Info::State)));
    connect(m_info, SIGNAL(started()), this, SLOT(opened()));
    connect(m_info, SIGNAL(stopped()), this, SLOT(closed()));
    connect(m_info, SIGNAL(removed()), this, SLOT(removed()));

    connect(m_activityConsumer, SIGNAL(currentActivityChanged(QString)), this, SLOT(checkIfCurrent()));
    checkIfCurrent();

    //find your containments
    foreach (Plasma::Containment *cont, PlasmaApp::self()->corona()->containments()) {
        if ((cont->containmentType() == Plasma::Containment::DesktopContainment ||
             cont->containmentType() == Plasma::Containment::CustomContainment) &&
            !PlasmaApp::self()->corona()->offscreenWidgets().contains(cont) && cont->context()->currentActivityId() == id) {
            insertContainment(cont);
        }
    }

    //qDebug() << m_containments.size();
}

Activity::~Activity()
{
}

void Activity::activityChanged()
{
    setName(m_info->name());
    setIcon(m_info->icon());
}

void Activity::activityStateChanged(KActivities::Info::State state)
{
    Q_UNUSED(state)
    emit stateChanged();
}

QString Activity::id()
{
    return m_id;
}

QString Activity::name()
{
    return m_name;
}

QPixmap Activity::pixmap(const QSize &size)
{
    if (m_info->isValid() && !m_info->icon().isEmpty()) {
        return KIcon(m_info->icon()).pixmap(size);
    } else {
        return KIdenticonGenerator::self()->generatePixmap(size.width(), m_id);
    }
}

bool Activity::isCurrent()
{
    return m_current;
    //TODO maybe plasmaapp should cache the current activity to reduce dbus calls?
}

void Activity::checkIfCurrent()
{
    const bool current = m_id == m_activityConsumer->currentActivity();
    if (current != m_current) {
        m_current = current;
        emit currentStatusChanged();
    }
}

KActivities::Info::State Activity::state()
{
    return m_info->state();
}

void Activity::remove()
{
    KActivities::Controller().removeActivity(m_id);
}

void Activity::removed()
{
    if (! m_containments.isEmpty()) {
        //FIXME only PlasmaApp::self()->corona() has authority to remove properly
        qDebug() << "!!!!! if your widgets are locked you've hit a BUG now";
        foreach (Plasma::Containment *c, m_containments) {
            c->destroy(false);
        }
    }

    const QString name = "activities/" + m_id;
    QFile::remove(KStandardDirs::locateLocal("appdata", name));
}

Plasma::Containment* Activity::containmentForScreen(int screen, int desktop)
{
    Plasma::Containment *containment = m_containments.value(QPair<int,int>(screen, desktop));

    if (!containment) {
        qDebug() << "adding containment for" << screen << desktop;
        // first look to see if there are any unnasigned containments that are candidates for
        // being sucked into this Activity
        foreach (Plasma::Containment *c, PlasmaApp::self()->corona()->containments()) {
            if ((c->containmentType() == Plasma::Containment::DesktopContainment ||
                c->containmentType() == Plasma::Containment::CustomContainment) &&
                c->context()->currentActivityId().isEmpty() &&
                !PlasmaApp::self()->corona()->offscreenWidgets().contains(c) &&
                m_containments.key(c, QPair<int,int>(-2,-2)) == QPair<int,int>(-2,-2)) {
                containment = c;
                containment->setScreen(screen, desktop);
                break;
            }
        }

        if (!containment) {
            // we ask for the containment for the screen with a default plugin, because
            // this allows the corona to either grab one for us that already exists matching
            // screen and desktop, or create a new one. this also works regardless of immutability
            containment = PlasmaApp::self()->corona()->containmentForScreen(screen, desktop, m_plugin);

            if (!containment || !containment->context()->currentActivityId().isEmpty()) {
                // possibly a plugin failure, let's go for the default
                containment = PlasmaApp::self()->corona()->containmentForScreen(screen, desktop, "default");
            }

            //we don't want to steal contaiments from other activities
            if (!containment) {
                // we failed to even get the default; we're screwed.
                Q_ASSERT(false);
                return 0;
            }

            if (!containment->context()->currentActivityId().isEmpty() &&
                containment->context()->currentActivityId() != m_id) {
                // we got a containment, but it belongs to some other activity; let's unassign it
                // from a screen and grab a new one
                containment->setScreen(-1);
                containment = PlasmaApp::self()->corona()->containmentForScreen(screen, desktop, m_plugin);

                if (!containment) {
                    // possibly a plugin failure, let's go for the default
                    containment = PlasmaApp::self()->corona()->containmentForScreen(screen, desktop, "default");
                }

                if (containment) {
                    containment->setScreen(screen, desktop);
                }
            }
        }

        if (containment) {
            insertContainment(containment, screen, desktop);
            PlasmaApp::self()->corona()->requestConfigSync();
        }
    } else if (containment->screen() != screen || containment->desktop() != desktop) {
        // ensure the containment _also_ knows which screen we think it is on;
        // can happen when swapping between activities without stopping them first
        containment->setScreen(screen, desktop);
    }

    return containment;
}

void Activity::activate()
{
    KActivities::Controller().setCurrentActivity(m_id);
}

void Activity::ensureActive()
{
    if (m_containments.isEmpty()) {
        opened();
    }

    checkScreens();
}

void Activity::checkScreens()
{
    //ensure there's a containment for every screen & desktop.
    int numScreens = PlasmaApp::self()->corona()->numScreens();
    int numDesktops = AppSettings::perVirtualDesktopViews() ? KWindowSystem::numberOfDesktops() : 0;

    for (int screen = 0; screen < numScreens; ++screen) {
        if (numDesktops > 0) {
            for (int desktop = 0; desktop < numDesktops; ++desktop) {
                containmentForScreen(screen, desktop);
            }
        } else {
            containmentForScreen(screen, -1);
        }
    }
}

void Activity::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;

    foreach (Plasma::Containment *c, m_containments) {
        c->context()->setCurrentActivity(name);
    }
}

void Activity::setIcon(const QString &icon)
{
    if (m_icon == icon) {
        return;
    }

    m_icon = icon;
}

void Activity::save(KConfig &external)
{
    foreach (const QString &group, external.groupList()) {
        KConfigGroup cg(&external, group);
        cg.deleteGroup();
    }

    //TODO: multi-screen saving/restoring, where each screen can be
    // independently restored: put each screen's containments into a
    // different group, e.g. [Screens][0][Containments], [Screens][1][Containments], etc
    KConfigGroup dest(&external, "Containments");
    KConfigGroup dummy;
    foreach (Plasma::Containment *c, m_containments) {
        c->save(dummy);
        KConfigGroup group(&dest, QString::number(c->id()));
        c->config().copyTo(&group);
    }

    external.sync();
}

void Activity::close()
{
    KActivities::Controller().stopActivity(m_id);
}

void Activity::closed()
{
    const QString name = "activities/" + m_id;
    KConfig external(name, KConfig::SimpleConfig, "appdata");

    //passing an empty string for the group name turns a kconfig into a kconfiggroup
    KConfigGroup group = external.group(QString());
    PlasmaApp::self()->corona()->exportLayout(group, m_containments.values());

    //hrm, shouldn't the containments' deleted signals have done this for us?
    if (!m_containments.isEmpty()) {
        qDebug() << "isn't close supposed to *remove* containments??";
        m_containments.clear();
    }
}

void Activity::replaceContainment(Plasma::Containment* containment)
{
    insertContainment(containment, true);
}

void Activity::insertContainment(Plasma::Containment* cont, bool force)
{
    int screen = cont->lastScreen();
    int desktop = cont->lastDesktop();

    qDebug() << screen << desktop;
    if (screen == -1) {
        //the migration can't set lastScreen, so maybe we need to assign the containment here
        qDebug() << "found a lost one";
        screen = 0;
    }

    if (!force && m_containments.contains(QPair<int,int>(screen, desktop))) {
        //this almost certainly means someone has been meddling where they shouldn't
        //but we should protect them from harm anyways
        qDebug() << "@!@!@!@!@!@@@@rejecting containment!!!";
        cont->context()->setCurrentActivityId(QString());
        return;
    }

    insertContainment(cont, screen, desktop);
}

void Activity::insertContainment(Plasma::Containment* containment, int screen, int desktop)
{
    //ensure it's hooked up
    Plasma::Context *context = containment->context();
    context->setCurrentActivityId(m_id);
    context->setCurrentActivity(m_name);

    m_containments.insert(QPair<int,int>(screen, desktop), containment);
    connect(containment, SIGNAL(destroyed(QObject*)), this, SLOT(containmentDestroyed(QObject*)));
}

void Activity::containmentDestroyed(QObject *object)
{
    //safe here because we are not accessing it
    Plasma::Containment *deletedCont = static_cast<Plasma::Containment *>(object);

    QHash<QPair<int,int>, Plasma::Containment*>::iterator i;
    for (i = m_containments.begin(); i != m_containments.end(); ++i) {
        Plasma::Containment *cont = i.value();
        if (cont == deletedCont) {
            m_containments.remove(i.key());
            break;
        }
    }
}

void Activity::open()
{
    KActivities::Controller().startActivity(m_id);
}

void Activity::opened()
{
    if (!m_containments.isEmpty()) {
        qDebug() << "already open!";
        return;
    }

    const QString fileName = KGlobal::dirs()->locateLocal("appdata", "activities/" + m_id);
    qDebug() << "&&&&&&&&&&&&&&&" << fileName;
    if (QFile::exists(fileName)) {
        {
            KConfig external(fileName, KConfig::SimpleConfig);

            foreach (Plasma::Containment *newContainment, PlasmaApp::self()->corona()->importLayout(external.group(QByteArray()))) {
                insertContainment(newContainment);
                //ensure it's hooked up (if something odd happened we don't want orphan containments)
                Plasma::Context *context = newContainment->context();
                context->setCurrentActivityId(m_id);
            }
        }

        QFile::remove(fileName);
    }

    if (m_containments.isEmpty()) {
        //TODO check if we need more for screens/desktops
        qDebug() << "open failed (bad file?). creating new containment";
        checkScreens();
    }

    PlasmaApp::self()->corona()->requireConfigSync();
}

void Activity::setDefaultPlugin(const QString &plugin)
{
    m_plugin = plugin;
    //FIXME save&restore this setting
}

const KActivities::Info * Activity::info() const
{
    return m_info;
}

#include "activity.moc"

// vim: sw=4 sts=4 et tw=100
