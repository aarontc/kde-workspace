/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
 *   Copyright (C) 2009 Matthieu Gallien <matthieu_gallien@yahoo.fr>       *
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

#include "statusnotifieritem_engine.h"
#include "statusnotifieritemsource.h"

#include <KDebug>
#include <KIcon>
#include <iostream>

StatusNotifierItemEngine::StatusNotifierItemEngine(QObject *parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args),
      m_dbus(QDBusConnection::sessionBus()),
      m_statusNotifierWatcher(0)
{
    Q_UNUSED(args);
}

StatusNotifierItemEngine::~StatusNotifierItemEngine()
{
    m_dbus.unregisterService(m_serviceName);
}

Plasma::Service* StatusNotifierItemEngine::serviceForSource(const QString &name)
{
    StatusNotifierItemSource *source = dynamic_cast<StatusNotifierItemSource*>(containerForSource(name));
    // if source does not exist, return null service
    if (!source) {
        return Plasma::DataEngine::serviceForSource(name);
    }

    Plasma::Service *service = source->createService();
    service->setParent(this);
    return service;
}

void StatusNotifierItemEngine::init()
{
    if (m_dbus.isConnected()) {
        QDBusConnectionInterface *dbusInterface = m_dbus.interface();

        m_serviceName = "org.kde.StatusNotifierHost-" + QString::number(QCoreApplication::applicationPid());
        m_dbus.registerService(m_serviceName);

        //FIXME: understand why registerWatcher/unregisterWatcher doesn't work
        /*connect(dbusInterface, SIGNAL(serviceRegistered(const QString&)),
            this, SLOT(registerWatcher(const QString&)));
        connect(dbusInterface, SIGNAL(serviceUnregistered(const QString&)),
            this, SLOT(unregisterWatcher(const QString&)));*/
        connect(dbusInterface, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                this, SLOT(serviceChange(QString,QString,QString)));

        registerWatcher("org.kde.StatusNotifierWatcher");
    }
}

void StatusNotifierItemEngine::serviceChange(const QString& name,
					      const QString& oldOwner,
					      const QString& newOwner)
{
    if (name != "org.kde.StatusNotifierWatcher") {
        return;
    }

    kDebug()<<"Service "<<name<<"status change, old owner:"<<oldOwner<<"new:"<<newOwner;

    if (newOwner.isEmpty()) {
        //unregistered
        unregisterWatcher(name);
    } else if (oldOwner.isEmpty()) {
        //registered
        registerWatcher(name);
    }
}

void StatusNotifierItemEngine::registerWatcher(const QString& service)
{
    kDebug()<<"service appeared"<<service;
    if (service == "org.kde.StatusNotifierWatcher") {
        QString interface("org.kde.StatusNotifierWatcher");
        if (m_statusNotifierWatcher) {
            delete m_statusNotifierWatcher;
        }

        m_statusNotifierWatcher = new org::kde::StatusNotifierWatcher(interface, "/StatusNotifierWatcher",
								      QDBusConnection::sessionBus());
        if (m_statusNotifierWatcher->isValid() &&
            m_statusNotifierWatcher->property("ProtocolVersion").toBool() == s_protocolVersion) {
            connect(m_statusNotifierWatcher, SIGNAL(StatusNotifierItemRegistered(const QString&)), this, SLOT(serviceRegistered(const QString &)));
            connect(m_statusNotifierWatcher, SIGNAL(StatusNotifierItemUnregistered(const QString&)), this, SLOT(serviceUnregistered(const QString&)));

            m_statusNotifierWatcher->call(QDBus::NoBlock, "RegisterStatusNotifierHost", m_serviceName);

            QStringList registeredItems = m_statusNotifierWatcher->property("RegisteredStatusNotifierItems").value<QStringList>();
            foreach (const QString &service, registeredItems) {
                newItem(service);
            }
        } else {
            delete m_statusNotifierWatcher;
            m_statusNotifierWatcher = 0;
            kDebug()<<"System tray daemon not reachable";
        }
    }
}

void StatusNotifierItemEngine::unregisterWatcher(const QString& service)
{
    if (service == "org.kde.StatusNotifierWatcher") {
        kDebug()<<"org.kde.StatusNotifierWatcher disappeared";

        disconnect(m_statusNotifierWatcher, SIGNAL(StatusNotifierItemRegistered(const QString&)), this, SLOT(serviceRegistered(const QString &)));
        disconnect(m_statusNotifierWatcher, SIGNAL(StatusNotifierItemUnregistered(const QString&)), this, SLOT(serviceUnregistered(const QString&)));

        removeAllSources();

        delete m_statusNotifierWatcher;
        m_statusNotifierWatcher = 0;
    }
}

void StatusNotifierItemEngine::serviceRegistered(const QString &service)
{
    kDebug() << "Registering"<<service;
    newItem(service);
}

void StatusNotifierItemEngine::serviceUnregistered(const QString &service)
{
    cleanupItem(service);
}

void StatusNotifierItemEngine::newItem(const QString &service)
{
    StatusNotifierItemSource *itemSource = new StatusNotifierItemSource(service, this);
    addSource(itemSource);
}

void StatusNotifierItemEngine::cleanupItem(const QString &service)
{
    removeSource(service);
}

K_EXPORT_PLASMA_DATAENGINE(statusnotifieritem, StatusNotifierItemEngine)

#include "statusnotifieritem_engine.moc"
