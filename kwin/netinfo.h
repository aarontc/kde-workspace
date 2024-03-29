/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009 Lucas Murray <lmurray@undefinedfire.com>
Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef KWIN_NETINFO_H
#define KWIN_NETINFO_H

#include <KDE/NETRootInfo>

#include <xcb/xcb.h>

namespace KWin
{

class Client;

/**
 * NET WM Protocol handler class
 */
class RootInfo : public NETRootInfo
{
private:
    typedef KWin::Client Client;  // Because of NET::Client

public:
    static RootInfo *create();
    static void destroy();

protected:
    virtual void changeNumberOfDesktops(int n) override;
    virtual void changeCurrentDesktop(int d) override;
    virtual void changeActiveWindow(Window w, NET::RequestSource src, Time timestamp, Window active_window) override;
    virtual void closeWindow(Window w) override;
    virtual void moveResize(Window w, int x_root, int y_root, unsigned long direction) override;
    virtual void moveResizeWindow(Window w, int flags, int x, int y, int width, int height) override;
    virtual void gotPing(Window w, Time timestamp) override;
    virtual void restackWindow(Window w, RequestSource source, Window above, int detail, Time timestamp) override;
    virtual void gotTakeActivity(Window w, Time timestamp, long flags) override;
    virtual void changeShowingDesktop(bool showing) override;

private:
    RootInfo(xcb_window_t w, const char* name, unsigned long pr[],
             int pr_num, int scr = -1);
    static RootInfo *s_self;
    friend RootInfo *rootInfo();
};

inline RootInfo *rootInfo()
{
    return RootInfo::s_self;
}

/**
 * NET WM Protocol handler class
 */
class WinInfo : public NETWinInfo2
{
private:
    typedef KWin::Client Client; // Because of NET::Client

public:
    WinInfo(Client* c, Display * display, Window window,
            Window rwin, const unsigned long pr[], int pr_size);
    virtual void changeDesktop(int desktop) override;
    virtual void changeFullscreenMonitors(NETFullscreenMonitors topology) override;
    virtual void changeState(unsigned long state, unsigned long mask) override;
    void disable();

private:
    Client * m_client;
};

} // KWin

#endif
