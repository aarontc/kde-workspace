/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "xinput_helper.h"

#include <kapplication.h>
#include <kdebug.h>

#include <QtGui/QX11Info>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#if HAVE_XINPUT==1
#include <X11/extensions/XInput.h>
#endif

#include "x11_helper.h"


static int DEVICE_NONE = 0;
static int DEVICE_KEYBOARD = 1;
static int DEVICE_POINTER = 2;

XInputEventNotifier::XInputEventNotifier(QWidget* parent):
	XEventNotifier(parent),
	xinputEventType(-1)
{
}

void XInputEventNotifier::start()
{
	if( KApplication::kApplication() != NULL ) {
		registerForNewDeviceEvent(QX11Info::display());
	}

	XEventNotifier::start();
}

void XInputEventNotifier::stop()
{
	XEventNotifier::stop();

	if( KApplication::kApplication() != NULL ) {
	//    XEventNotifier::unregisterForNewDeviceEvent(QX11Info::display());
	}
}

bool XInputEventNotifier::processOtherEvents(XEvent* event)
{
	int newDeviceType = getNewDeviceEventType(event);
	if( newDeviceType == DEVICE_KEYBOARD ) {
		emit(newKeyboardDevice());
	}
	else if( newDeviceType == DEVICE_POINTER ) {
		emit(newPointerDevice());
	}
	return true;
}


#if HAVE_XINPUT==1

extern "C" {
    extern int _XiGetDevicePresenceNotifyEvent(Display *);
}

int XInputEventNotifier::getNewDeviceEventType(XEvent* event)
{
	int newDeviceType = DEVICE_NONE;

	if( xinputEventType != -1 && event->type == xinputEventType ) {
		XDevicePresenceNotifyEvent *xdpne = (XDevicePresenceNotifyEvent*) event;
		if( xdpne->devchange == DeviceEnabled ) {
			int ndevices;
			XDeviceInfo	*devices = XListInputDevices(xdpne->display, &ndevices);
			if( devices != NULL ) {
				kDebug() << "New device id:" << xdpne->deviceid;
				for(int i=0; i<ndevices; i++) {
					kDebug() << "id:" << devices[i].id << "name:" << devices[i].name << "used as:" << devices[i].use;
					if( devices[i].id == xdpne->deviceid ) {
						if( devices[i].use == IsXKeyboard || devices[i].use == IsXExtensionKeyboard ) {
							newDeviceType = DEVICE_KEYBOARD;
							break;
						}
						if( devices[i].use == IsXPointer || devices[i].use == IsXExtensionPointer ) {
							newDeviceType = DEVICE_POINTER;
							break;
						}
					}
				}
				XFreeDeviceList(devices);
			}
		}
	}
	return newDeviceType;
}

int XInputEventNotifier::registerForNewDeviceEvent(Display* display)
{
	int xitype;
	XEventClass xiclass;

	DevicePresence(display, xitype, xiclass);
	XSelectExtensionEvent(display, DefaultRootWindow(display), &xiclass, 1);
	kDebug() << "Registered for new device events from XInput, class" << xitype;
	xinputEventType = xitype;
	return xitype;
}

#else

#warning "Keyboard daemon is compiled without XInput, keyboard settings will be reset when new keyboard device is plugged in!"

int XInputEventNotifier::registerForNewDeviceEvent(Display* /*display*/)
{
	kWarning() << "Keyboard kded daemon is compiled without XInput, xkb configuration will be reset when new keyboard device is plugged in!";
	return -1;
}

int XInputEventNotifier::getNewDeviceEventType(XEvent* /*event*/)
{
	return DEVICE_NONE;
}

#endif