/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	KTop is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

	$Id$
*/

#ifndef _MultiMeter_h_
#define _MultiMeter_h_

#include <SensorDisplay.h>

class QGroupBox;
class QLCDNumber;
class MultiMeterSettings;

class MultiMeter : public SensorDisplay
{
	Q_OBJECT

public:
	MultiMeter(QWidget* parent = 0, const char* name = 0,
			   const QString& = QString::null, int min = 0, int max = 0);
	~MultiMeter()
	{
	}

	bool addSensor(const QString& hostName, const QString& sensorName,
				   const QString& sensorDescr);
	void answerReceived(int id, const QString& answer);
	void resizeEvent(QResizeEvent*);

	bool load(QDomElement& domEl);
	bool save(QDomDocument& doc, QDomElement& display);

	void settings();

public slots:
	void applySettings();

private:
	void setTitle(const QString& t, const QString& u);

	QGroupBox* frame;
	QLCDNumber* lcd;
	QString title;
	QString unit;
	bool modified;
	MultiMeterSettings* mms;
	bool showUnit;
	bool lowerLimitActive;
	long lowerLimit;
	bool upperLimitActive;
	long upperLimit;
} ;

#endif
