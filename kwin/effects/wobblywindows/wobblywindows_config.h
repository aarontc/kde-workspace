/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2008 Cédric Borgese <cedric.borgese@gmail.com>
 Copyright (C) 2008 Lucas Murray <lmurray@undefinedfire.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef KWIN_WOBBLYWINDOWS_CONFIG_H
#define KWIN_WOBBLYWINDOWS_CONFIG_H

#include <kcmodule.h>

#include "ui_wobblywindows_config.h"
#include "wobblywindows.h"


namespace KWin
{

class WobblyWindowsEffectConfig : public KCModule
{
    Q_OBJECT
public:
    explicit WobblyWindowsEffectConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());
    ~WobblyWindowsEffectConfig();

public Q_SLOTS:
    virtual void save();

private Q_SLOTS:
    void wobblinessChanged();

private:
    ::Ui::WobblyWindowsEffectConfigForm m_ui;
};

} // namespace

#endif
