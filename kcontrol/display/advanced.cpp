/* vi: ts=8 sts=4 sw=4
 *
 * $Id: $
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <qobject.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include <kglobal.h>
#include <klocale.h>
#include <kwm.h>

#include <bgdefaults.h>
#include "backgnd.h"
#include "advanced.h"


/**** KAdvanced ****/

KAdvanced::KAdvanced(QWidget *parent, Mode m)
    : KDisplayModule(parent, m)
{
    if (m == Init)
	return;

    // Top layout
    QVBoxLayout *top = new QVBoxLayout(this);
    top->setMargin(10);
    top->setSpacing(10);

    // Advanced background settings
    QGroupBox *group = new QGroupBox(i18n("Background"), this);
    top->addWidget(group);
    QVBoxLayout *vbox = new QVBoxLayout(group);
    vbox->setSpacing(10);
    vbox->setMargin(10);
    vbox->addSpacing(10);

    m_pCBLimit = new QCheckBox(i18n("&Limit Pixmap Cache"), group);
    vbox->addWidget(m_pCBLimit);
    connect(m_pCBLimit, SIGNAL(toggled(bool)), SLOT(slotLimitCache(bool)));

    QHBoxLayout *hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    QLabel *lbl = new QLabel(i18n("Cache &Size"), group);
    lbl->setFixedSize(lbl->sizeHint());
    hbox->addSpacing(20);
    hbox->addWidget(lbl);
    m_pCacheBox = new QSpinBox(group);
    m_pCacheBox->setSteps(512, 1024);
    m_pCacheBox->setSuffix(i18n(" Kb"));
    m_pCacheBox->setRange(512, 10240);
    lbl->setBuddy(m_pCacheBox);
    connect(m_pCacheBox, SIGNAL(valueChanged(int)), SLOT(slotCacheSize(int)));
    hbox->addWidget(m_pCacheBox);
    hbox->addStretch();

    m_pCBExport = new QCheckBox(i18n("&Export Background to shared Pixmap"), group);
    vbox->addWidget(m_pCBExport);
    connect(m_pCBExport, SIGNAL(toggled(bool)), SLOT(slotExportBackground(bool)));

    m_pCBDock = new QCheckBox(i18n("Doc&k to panel"), group);
    vbox->addWidget(m_pCBDock);
    connect(m_pCBDock, SIGNAL(toggled(bool)), SLOT(slotDockPanel(bool)));

    top->addStretch();

    m_pSettings = new KGlobalBackgroundSettings();
    apply();

}


void KAdvanced::apply()
{
    if (m_pSettings->limitCache()) {
	m_pCBLimit->setChecked(true);
	m_pCacheBox->setEnabled(true);
    } else {
	m_pCBLimit->setChecked(false);
	m_pCacheBox->setEnabled(false);
    }

    m_pCacheBox->setValue(m_pSettings->cacheSize());
    m_pCBExport->setChecked(m_pSettings->exportBackground());
    m_pCBDock->setChecked(m_pSettings->dockPanel());
}


void KAdvanced::slotLimitCache(bool limit)
{
    m_pSettings->setLimitCache(limit);
    apply();
}


void KAdvanced::slotCacheSize(int size)
{
    m_pSettings->setCacheSize(size);
    apply();
}


void KAdvanced::slotExportBackground(bool exp)
{
    m_pSettings->setExportBackground(exp);
    apply();
}


void KAdvanced::slotDockPanel(bool dock)
{
    m_pSettings->setDockPanel(dock);
    apply();
}


void KAdvanced::loadSettings()
{
    qDebug("KAdvanced::loadSettings");
}


void KAdvanced::applySettings()
{
    m_pSettings->writeSettings();
    KWM::sendKWMCommand("kbgwm_reconfigure");
}


void KAdvanced::defaultSettings()
{
    m_pSettings->setLimitCache(_defLimitCache);
    m_pSettings->setExportBackground(_defExport);
    m_pSettings->setDockPanel(_defDock);
    m_pSettings->setCacheSize(_defCacheSize);
    apply();
}
