/*
 * Copyright (c) 2002,2003 Hamish Rodda <meddie@yoyo.its.monash.edu.au>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qdesktopwidget.h>

#include <kdebug.h>
#include <klocale.h>
#include <kcmodule.h>
#include <kglobal.h>
#include <kgenericfactory.h>
#include <kcombobox.h>
#include <kdialog.h>

#include "krandrmodule.h"
#include "krandrmodule.moc"

// DLL Interface for kcontrol
typedef KGenericFactory<KRandRModule, QWidget > KSSFactory;
K_EXPORT_COMPONENT_FACTORY (libkcm_randr, KSSFactory("krandr") );

extern "C"
{
	void init_randr()
	{
#ifndef XRANDR_STARTUP_HACK
		KRandRModule::performApplyOnStartup();
#endif
	}
}

void KRandRModule::performApplyOnStartup()
{
	KConfig config("kcmrandrrc", true);
	if (RandRDisplay::applyOnStartup(config))
	{
		// Load settings and apply appropriate config
		RandRDisplay display;
		if (display.loadDisplay(config))
			display.applyProposed(false);
	}
}

KRandRModule::KRandRModule(QWidget *parent, const char *name, const QStringList&)
    : KCModule(parent, name)
	, m_changed(false)
{
	if (!isValid()) {
		QVBoxLayout *topLayout = new QVBoxLayout(this);
		topLayout->addWidget(new QLabel(i18n("<qt>Sorry, your X server does not support resizing and rotating the display. Please update to version 4.3 or greater.  You need the X Resize And Rotate extension (RANDR) version 1.1 or greater to use this feature.  Error code: %1</qt>").arg(errorCode()), this));
		return;
	}

	QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

	QHBox* screenBox = new QHBox(this);
	topLayout->addWidget(screenBox);
	new QLabel(i18n("Settings for screen:"), screenBox);
	m_screenSelector = new KComboBox(screenBox);

	for (int s = 0; s < numScreens(); s++) {
		m_screenSelector->insertItem(i18n("Screen %1").arg(s+1));
	}

	m_screenSelector->setCurrentItem(currentScreenIndex());

	connect(m_screenSelector, SIGNAL(activated(int)), SLOT(slotScreenChanged(int)));

	if (numScreens() <= 1)
		m_screenSelector->setEnabled(false);

	QHBox* sizeBox = new QHBox(this);
	topLayout->addWidget(sizeBox);
	new QLabel(i18n("Screen size:"), sizeBox);
	m_sizeCombo = new KComboBox(sizeBox);
	connect(m_sizeCombo, SIGNAL(activated(int)), SLOT(slotSizeChanged(int)));

	QHBox* refreshBox = new QHBox(this);
	topLayout->addWidget(refreshBox);
	new QLabel(i18n("Refresh rate:"), refreshBox);
	m_refreshRates = new KComboBox(refreshBox);
	connect(m_refreshRates, SIGNAL(activated(int)), SLOT(slotRefreshChanged(int)));

	m_rotationGroup = new QButtonGroup(2, Qt::Horizontal, i18n("Orientation (degrees counterclockwise)"), this);
	topLayout->addWidget(m_rotationGroup);
	m_rotationGroup->setRadioButtonExclusive(true);

	m_applyOnStartup = new QCheckBox(i18n("Apply settings on KDE startup"), this);
	topLayout->addWidget(m_applyOnStartup);
	connect(m_applyOnStartup, SIGNAL(clicked()), SLOT(setChanged()));

	QHBox* syncBox = new QHBox(this);
	syncBox->layout()->addItem(new QSpacerItem(20, 1, QSizePolicy::Maximum));
	m_syncTrayApp = new QCheckBox(i18n("Allow tray application to change startup settings"), syncBox);
	topLayout->addWidget(syncBox);
	connect(m_syncTrayApp, SIGNAL(clicked()), SLOT(setChanged()));

	topLayout->addStretch(1);

	// just set the "apply settings on startup" box
	load();
	m_syncTrayApp->setEnabled(m_applyOnStartup->isChecked());

	slotScreenChanged(QApplication::desktop()->primaryScreen());

	setButtons(KCModule::Apply);
}

void KRandRModule::addRotationButton(int thisRotation, bool checkbox)
{
	Q_ASSERT(m_rotationGroup);
	if (!checkbox) {
		QRadioButton* thisButton = new QRadioButton(RandRScreen::rotationName(thisRotation), m_rotationGroup);
		thisButton->setEnabled(thisRotation & currentScreen()->rotations());
		connect(thisButton, SIGNAL(clicked()), SLOT(slotRotationChanged()));
	} else {
		QCheckBox* thisButton = new QCheckBox(RandRScreen::rotationName(thisRotation), m_rotationGroup);
		thisButton->setEnabled(thisRotation & currentScreen()->rotations());
		connect(thisButton, SIGNAL(clicked()), SLOT(slotRotationChanged()));
	}
}

void KRandRModule::slotScreenChanged(int screen)
{
	setCurrentScreen(screen);

	// Clear resolutions
	m_sizeCombo->clear();

	// Add new resolutions
	for (int i = 0; i < currentScreen()->numSizes(); i++) {
		m_sizeCombo->insertItem(i18n("%1 x %2").arg(currentScreen()->pixelSize(i).width()).arg(currentScreen()->pixelSize(i).height()));

		// Aspect ratio
		/* , aspect ratio %5)*/
		/*.arg((double)currentScreen()->size(i).mwidth / (double)currentScreen()->size(i).mheight))*/
	}

	// Clear rotations
	for (int i = m_rotationGroup->count() - 1; i >= 0; i--)
		m_rotationGroup->remove(m_rotationGroup->find(i));

	// Create rotations
	for (int i = 0; i < RandRScreen::OrientationCount; i++)
		addRotationButton(1 << i, i > RandRScreen::RotationCount - 1);

	populateRefreshRates();

	update();

	setChanged();
}

void KRandRModule::slotRotationChanged()
{
	if (m_rotationGroup->find(0)->isOn())
		currentScreen()->proposeRotation(RandRScreen::Rotate0);
	else if (m_rotationGroup->find(1)->isOn())
		currentScreen()->proposeRotation(RandRScreen::Rotate90);
	else if (m_rotationGroup->find(2)->isOn())
		currentScreen()->proposeRotation(RandRScreen::Rotate180);
	else {
		Q_ASSERT(m_rotationGroup->find(3)->isOn());
		currentScreen()->proposeRotation(RandRScreen::Rotate270);
	}

	if (m_rotationGroup->find(4)->isOn())
		currentScreen()->proposeRotation(RandRScreen::ReflectX);

	if (m_rotationGroup->find(5)->isOn())
		currentScreen()->proposeRotation(RandRScreen::ReflectY);

	setChanged();
}

void KRandRModule::slotSizeChanged(int index)
{
	int oldProposed = currentScreen()->proposedSize();

	currentScreen()->proposeSize(index);

	if (currentScreen()->proposedSize() != oldProposed) {
		currentScreen()->proposeRefreshRate(0);

		populateRefreshRates();

		// Item with index zero is already selected
	}

	setChanged();
}

void KRandRModule::slotRefreshChanged(int index)
{
	currentScreen()->proposeRefreshRate(index);

	setChanged();
}

void KRandRModule::populateRefreshRates()
{
	m_refreshRates->clear();

	QStringList rr = currentScreen()->refreshRates(currentScreen()->proposedSize());

	for (QStringList::Iterator it = rr.begin(); it != rr.end(); it++)
		m_refreshRates->insertItem(*it);
}


void KRandRModule::defaults()
{
	if (currentScreen()->changedFromOriginal()) {
		currentScreen()->proposeOriginal();
		currentScreen()->applyProposed();
	} else {
		currentScreen()->proposeOriginal();
	}

	update();
}

void KRandRModule::load()
{
	// Don't load screen configurations:
	// It will be correct already if they wanted to retain their settings over KDE restarts,
	// and if it isn't correct they have changed a) their X configuration, b) the screen
	// with another program, or c) their hardware.
	KConfig config("kcmrandrrc", true);
	m_oldApply = loadDisplay(config, false);
	m_oldSyncTrayApp = syncTrayApp(config);

	m_applyOnStartup->setChecked(m_oldApply);
	m_syncTrayApp->setChecked(m_oldSyncTrayApp);

	setChanged();
}

void KRandRModule::save()
{
	apply();

	m_oldApply = m_applyOnStartup->isChecked();
	m_oldSyncTrayApp = m_syncTrayApp->isChecked();
	KConfig config("kcmrandrrc");
	saveDisplay(config, m_oldApply, m_oldSyncTrayApp);

	setChanged();
}

void KRandRModule::setChanged()
{
	bool isChanged = (m_oldApply != m_applyOnStartup->isChecked()) || (m_oldSyncTrayApp != m_syncTrayApp->isChecked());
	m_syncTrayApp->setEnabled(m_applyOnStartup->isChecked());

	if (!isChanged)
		for (int screenIndex = 0; screenIndex < numScreens(); screenIndex++) {
			if (screen(screenIndex)->proposedChanged()) {
				isChanged = true;
				break;
			}
		}

	if (isChanged != m_changed) {
		m_changed = isChanged;
		emit changed(m_changed);
	}
}

void KRandRModule::apply()
{
	if (m_changed) {
		applyProposed();

		update();
	}
}


void KRandRModule::update()
{
	m_sizeCombo->blockSignals(true);
	m_sizeCombo->setCurrentItem(currentScreen()->proposedSize());
	m_sizeCombo->blockSignals(false);

	m_rotationGroup->blockSignals(true);
	switch (currentScreen()->proposedRotation() & RandRScreen::RotateMask) {
		case RandRScreen::Rotate0:
			m_rotationGroup->setButton(0);
			break;
		case RandRScreen::Rotate90:
			m_rotationGroup->setButton(1);
			break;
		case RandRScreen::Rotate180:
			m_rotationGroup->setButton(2);
			break;
		case RandRScreen::Rotate270:
			m_rotationGroup->setButton(3);
			break;
		default:
			// Shouldn't hit this one
			Q_ASSERT(currentScreen()->proposedRotation() & RandRScreen::RotateMask);
			break;
	}
	m_rotationGroup->find(4)->setDown(currentScreen()->proposedRotation() & RandRScreen::ReflectX);
	m_rotationGroup->find(5)->setDown(currentScreen()->proposedRotation() & RandRScreen::ReflectY);
	m_rotationGroup->blockSignals(false);

	m_refreshRates->blockSignals(true);
	m_refreshRates->setCurrentItem(currentScreen()->proposedRefreshRate());
	m_refreshRates->blockSignals(false);
}
