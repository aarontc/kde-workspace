/*
    KTop, the KDE Task Manager and System Monitor
   
	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

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

#include <assert.h>

#include <qevent.h>
#include <qdragobject.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "SensorBrowser.h"
#include "SensorManager.h"
#include "SensorBrowser.moc"

SensorBrowser::SensorBrowser(QWidget* parent, SensorManager* sm,
							 const char* name) :
	QListView(parent, name), sensorManager(sm)
{
	connect(sm, SIGNAL(update(void)), this, SLOT(update(void)));

	addColumn(i18n("Sensor Browser"));
	setRootIsDecorated(TRUE);

	// Fill the sensor description dictionary.
	dict.insert("cpuidle", new QString(i18n("CPU Idle Load")));
	dict.insert("cpusys", new QString(i18n("CPU System Load")));
	dict.insert("cpunice", new QString(i18n("CPU Nice Load")));
	dict.insert("cpuuser", new QString(i18n("CPU User Load")));
	dict.insert("memswap", new QString(i18n("Swap Memory")));
	dict.insert("memcached", new QString(i18n("Cached Memory")));
	dict.insert("membuf", new QString(i18n("Buffered Memory")));
	dict.insert("memused", new QString(i18n("Used Memory")));
	dict.insert("memfree", new QString(i18n("Free Memory")));
	dict.insert("pscount", new QString(i18n("Process Count")));
}

void
SensorBrowser::update()
{
	SensorManagerIterator it(sensorManager);

	hostInfos.clear();
	SensorAgent* host;
	for (int i = 0 ; (host = it.current()); ++it, ++i)
	{
		QString hostName = sensorManager-> getHostName(host);

		QListViewItem* lvi = new QListViewItem(this, hostName);
		CHECK_PTR(lvi);

		HostInfo* hostInfo = new HostInfo(hostName, lvi);
		CHECK_PTR(hostInfo);
		hostInfos.append(hostInfo);

		// request sensor list from host
		host->sendRequest("monitors", this, i);
	}
}

void
SensorBrowser::answerReceived(int id, const QString& s)
{
	/* An answer has the following format:

	   cpuidle	integer
	   cpusys 	integer
	   cpunice	integer
	   cpuuser	integer
	   ps	table
	*/

	SensorTokenizer lines(s, '\n');

	for (unsigned int i = 0; i < lines.numberOfTokens(); ++i)
	{
		SensorTokenizer words(lines[i], '\t');

		QString sensorName = words[0];
		QString sensorType = words[1];

		// retrieve localized description from dictionary
		QString sensorDescription;
		if (!dict[sensorName])
			sensorDescription = sensorName;
		else
			sensorDescription = *(dict[sensorName]);

		QListViewItem* lvi = new QListViewItem(hostInfos.at(id)->getLVI(),
											   sensorDescription);
		CHECK_PTR(lvi);

		// add sensor info to internal data structure
		hostInfos.at(id)->addSensor(lvi, sensorName, sensorDescription,
									sensorType);
	}
}

void
SensorBrowser::viewportMouseMoveEvent(QMouseEvent* ev)
{
	QListViewItem* item = itemAt(ev->pos());
	if (!item)
		return;		// no item under cursor

	QListViewItem* parent = item->parent();
	if (!parent)
		return;		// item is not a sensor name

	// find the host info record that belongs to the LVI
	QListIterator<HostInfo> it(hostInfos);
	for ( ; it.current() && (*it)->getLVI() != parent; ++it)
		;
	assert(it.current());

	// Create text drag object as "<hostname> <sensorname>".
	dragText = (*it)->getHostName() + " " + (*it)->getSensorName(item);

	QDragObject* dObj = new QTextDrag(dragText, this);
	CHECK_PTR(dObj);
	dObj->dragCopy();
}
