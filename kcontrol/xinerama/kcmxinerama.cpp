/**
 * kcmxinerama.cpp
 *
 * Copyright (c) 2002-2003 George Staikos <staikos@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "kcmxinerama.h"
#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <qcheckbox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <kapplication.h>
#include <qtable.h>
#include <qcolor.h>
#include <kwin.h>
#include <qpushbutton.h>


KCMXinerama::KCMXinerama(QWidget *parent, const char *name)
  : KCModule(parent, name) {
	_indicators.setAutoDelete(true);

	config = new KConfig("xinerama", false, false);

	connect(&_timer, SIGNAL(timeout()), this, SLOT(clearIndicator()));

	QGridLayout *grid = new QGridLayout(this, 1, 1, KDialog::marginHint(),
							KDialog::spacingHint());

	// Setup the panel
	_displays = QApplication::desktop()->numScreens();

	if (_displays > 1) {
		QStringList dpyList;
		xw = new XineramaWidget(this);
		grid->addWidget(xw, 0, 0);
		QString label = i18n("Display %1");

		xw->headTable->setNumRows(_displays);

		for (int i = 0; i < _displays; i++) {
			QString l = label.arg(i+1);
			QRect geom = QApplication::desktop()->screenGeometry(i);
			xw->_kdmDisplay->insertItem(l);
			xw->_unmanagedDisplay->insertItem(l);
			dpyList.append(l);
			xw->headTable->setText(i, 0, QString::number(geom.x()));
			xw->headTable->setText(i, 1, QString::number(geom.y()));
			xw->headTable->setText(i, 2, QString::number(geom.width()));
			xw->headTable->setText(i, 3, QString::number(geom.height()));
		}

		xw->headTable->setRowLabels(dpyList);

		connect(xw->_kdmDisplay, SIGNAL(activated(int)),
			this, SLOT(windowIndicator(int)));
		connect(xw->_unmanagedDisplay, SIGNAL(activated(int)),
			this, SLOT(windowIndicator(int)));
		connect(xw->_identify, SIGNAL(clicked()),
			this, SLOT(indicateWindows()));
	} else { // no Xinerama
		QLabel *ql = new QLabel(i18n("<qt><p>This module is only for configuring systems with a single desktop spread across multiple monitors. You do not appear to have this configuration.</p></qt>"), this);
		grid->addWidget(ql, 0, 0);
	}

	grid->activate();

	load();
}

KCMXinerama::~KCMXinerama() {
	_timer.stop();
	delete config;
	config = 0;
	clearIndicator();
}

void KCMXinerama::configChanged() {
	emit changed(true);
}


void KCMXinerama::load() {
	if (_displays > 1) {
	}
	emit changed(false);
}


void KCMXinerama::save() {
	if (_displays > 1) {
		config->sync();
	}
	emit changed(false);
}

void KCMXinerama::defaults() {
	emit changed(true);
}

void KCMXinerama::indicateWindows() {
	_timer.stop();

	clearIndicator();
	for (int i = 0; i < _displays; i++)
		_indicators.append(indicator(i));

	_timer.start(1500, true);
}

void KCMXinerama::windowIndicator(int dpy) {
	_timer.stop();

	clearIndicator();
	_indicators.append(indicator(dpy));

	_timer.start(1500, true);
}

QWidget *KCMXinerama::indicator(int dpy) {
	QRect r = QApplication::desktop()->screenGeometry(dpy);

	r.setLeft(r.left() + 5*r.width()/12);
	r.setRight(r.left() + 1*r.width()/6);
	r.setTop(r.top() + 5*r.height()/12);
	r.setBottom(r.top() + 1*r.height()/6);

	QWidget *_screenIndicator = new QWidget(0, "Screen Indicator");
	_screenIndicator->setGeometry(r);

	QLabel *l = new QLabel(QString::number(dpy+1), _screenIndicator);
	QFont fnt;
	fnt.setPixelSize(48);
	fnt.setBold(true);
	l->setFont(fnt);
	l->setPaletteForegroundColor(QColor(0,0xff,0));
	l->setBackgroundColor(QColor(0,0,0));
	l->resize(l->minimumSizeHint());
	l->move(QPoint((_screenIndicator->width() - l->width())/2, (_screenIndicator->height() - l->height())/2));

	_screenIndicator->showFullScreen();
	_screenIndicator->setGeometry(r);
	_screenIndicator->setBackgroundColor(QColor(0,0,0));
	KWin::setOnAllDesktops(_screenIndicator->winId(), true);
	KWin::setState(_screenIndicator->winId(), NET::StaysOnTop);
	_screenIndicator->show();

	return _screenIndicator;
}

void KCMXinerama::clearIndicator() {
	_indicators.clear();
}

QString KCMXinerama::quickHelp() const {
	return i18n("<h1>Multiple Monitors</h1> This module allows you to configure KDE support"
     " for multiple monitors.");
}

const KAboutData* KCMXinerama::aboutData() const {
 
	KAboutData *about =
	new KAboutData(I18N_NOOP("kcmxinerama"), I18N_NOOP("KDE Multiple Monitor Configurator"),
		0, 0, KAboutData::License_GPL,
		I18N_NOOP("(c) 2002-2003 George Staikos"));
 
	about->addAuthor("George Staikos", 0, "staikos@kde.org");
 
	return about;
}

extern "C" {
KCModule *create_xinerama(QWidget *parent, const char *name) {
	KGlobal::locale()->insertCatalogue("kcmxinerama");
	return new KCMXinerama(parent, name);
}

void init_xinerama() {
}
}


#include "kcmxinerama.moc"

