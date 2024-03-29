/***************************************************************************
 *   plasmoidtask.h                                                        *
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

#ifndef PLASMOIDTASK_H
#define PLASMOIDTASK_H

#include "../../core/task.h"

#include <plasma/plasma.h>

namespace SystemTray
{


class PlasmoidTask : public Task
{
    Q_OBJECT

public:
    PlasmoidTask(const QString &appletName, int id, QObject *parent, Plasma::Applet *host);
    virtual ~PlasmoidTask();

    bool isValid() const;
    virtual bool isEmbeddable() const;
    virtual QString taskId() const;
    virtual QIcon icon() const;
    void forwardConstraintsEvent(Plasma::Types::Constraints constraints);
    int id() const;
    Plasma::Applet *host() const;
    virtual bool isWidget() const;
    virtual TaskType type() const { return TypePlasmoid; }

protected Q_SLOTS:
    void appletDestroyed(Plasma::Applet *object);
    void newAppletStatus(Plasma::Types::ItemStatus status);

Q_SIGNALS:
    void taskDeleted(Plasma::Applet *host, const QString &taskId);

protected:
    virtual QQuickItem* createWidget(Plasma::Applet *applet);

private:
    void setupApplet(const QString &plugin, int id);

    QString m_appletName;
    QString m_taskId;
    QIcon m_icon;
    QWeakPointer<Plasma::Applet> m_applet;
    Plasma::Applet *m_host;
    bool m_takenByParent;
};

}

#endif
