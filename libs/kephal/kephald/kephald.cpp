/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
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


#include <QDebug>
#include <QDBusConnection>


#include "kephald.h"
#include "outputs/desktopwidget/desktopwidgetoutputs.h"
#include "screens/output/outputscreens.h"
#include "outputs/xrandr/xrandroutputs.h"
#include "dbus/dbusapi_screens.h"
#include "dbus/dbusapi_outputs.h"
//#include "xml/configurations_xml.h"
#include "configurations/xml/xmlconfigurations.h"
#include "xrandr12/randrdisplay.h"


using namespace kephal;


int main(int argc, char *argv[])
{
    KephalD app(argc, argv);

    return app.exec();
}


KephalD::KephalD(int & argc, char ** argv)
    : QApplication(argc, argv),
    noXRandR(false)
{
    qDebug() << "kephald starting up";
    parseArgs(argc, argv);
    init();
}

KephalD::~KephalD()
{
}

void KephalD::parseArgs(int & argc, char ** argv) {
    for (int i = 0; i < argc; ++i) {
        QString arg(argv[i]);
        qDebug() << "arg:" << i << arg;
        
        if (arg == "--no-xrandr") {
            noXRandR = true;
        }
    }
}

void KephalD::init() {
    RandRDisplay * display;
    if (! noXRandR) {
        display = new RandRDisplay();
    }
    
    if (! noXRandR && display->isValid()) {
        new XRandROutputs(this, display);
    } else {
        new DesktopWidgetOutputs(this);
    }
    
    foreach (Output * output, Outputs::instance()->outputs()) {
        qDebug() << "output:" << output->id() << output->geom();
    }
    
    new OutputScreens(this);
    
    foreach (kephal::Screen * screen, Screens::instance()->screens()) {
        qDebug() << "screen:" << screen->id() << screen->geom();
    }
    
    Configurations * configs = new XMLConfigurations(this);
    configs->findConfiguration()->activate();
    
    QDBusConnection dbus = QDBusConnection::sessionBus();
    bool result = dbus.registerService("org.kde.Kephal");
    qDebug() << "registered the service:" << result;
    
    new DBusAPIScreens(this);
    new DBusAPIOutputs(this);
}


