/***************************************************************************
 *   fdonotification.h                                                     *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#ifndef FDONOTIFICATION_H
#define FDONOTIFICATION_H

#include "../../core/notification.h"


namespace SystemTray
{

class FdoNotification : public Notification
{
    Q_OBJECT

    friend class FdoSelectionManager;
    friend class FdoSelectionManagerPrivate;

public:
    FdoNotification(WId winId, QObject *parent = 0);
    ~FdoNotification();

Q_SIGNALS:
    void notificationDeleted(WId winId);

private:
    WId m_winId;
};

}


#endif
