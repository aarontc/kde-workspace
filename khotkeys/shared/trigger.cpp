/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "triggers.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>

namespace KHotKeys {

void Trigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" );
    }

Trigger* Trigger::create_cfg_read( KConfigGroup& cfg_P, Action_data* data_P )
    {
    QString type = cfg_P.readEntry( "Type" );
    if( type == "SHORTCUT" || type == "SINGLE_SHORTCUT" )
        return new Shortcut_trigger( cfg_P, data_P );
    if( type == "WINDOW" )
        return new Window_trigger( cfg_P, data_P );
    if( type == "GESTURE" )
        return new Gesture_trigger(cfg_P, data_P );
    if( type == "VOICE" )
        return new Voice_trigger (cfg_P, data_P );

    kWarning( 1217 ) << "Unknown Trigger type read from cfg file\n";
    return NULL;
    }


} // namespace KHotKeys

