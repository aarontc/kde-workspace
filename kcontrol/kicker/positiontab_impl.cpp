/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2002 Aaron Seigo <aseigo@olympusproject.org>
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
 */
#include <stdlib.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qslider.h>
#include <qspinbox.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kpixmap.h>
#include <kstandarddirs.h>

#include "../background/bgrender.h"
#include "positiontab_impl.h"
#include "positiontab_impl.moc"


extern int kickerconfig_screen_number;

// magic numbers for the preview widget layout
extern const int offsetX = 23;
extern const int offsetY = 14;
extern const int maxX = 151;
extern const int maxY = 115;
extern const int margin = 1;

PositionTab::PositionTab( QWidget *parent, const char* name )
  : PositionTabBase (parent, name),
    m_pretendPanel(0),
    m_desktopPreview(0),
    m_panelPos(PosBottom),
    m_panelAlign(AlignLeft)
{
    QPixmap monitor(locate("data", "kcontrol/pics/monitor.png"));
    m_monitorImage->setPixmap(monitor);
    m_monitorImage->setFixedSize(m_monitorImage->sizeHint());

    m_pretendDesktop = new QWidget(m_monitorImage, "pretendBG");
    m_pretendDesktop->setGeometry(offsetX, offsetY, maxX, maxY);
    m_pretendPanel = new QFrame(m_monitorImage, "pretendPanel");
    m_pretendPanel->setGeometry(offsetX + margin, maxY + offsetY - 10, 
                                maxX - margin, 10 - margin);
    m_pretendPanel->setFrameShape(QFrame::MenuBarPanel);

    // connections
    connect(m_locationGroup, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_percentSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_percentSpinBox, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_expandCheckBox, SIGNAL(clicked()), SIGNAL(changed()));
    
    connect(m_sizeGroup, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_customSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_customSpinbox, SIGNAL(valueChanged(int)), SIGNAL(changed()));

    m_desktopPreview = new KBackgroundRenderer(0);
    connect(m_desktopPreview, SIGNAL(imageDone(int)), SLOT(slotBGPreviewReady(int)));
    load();
}

PositionTab::~PositionTab()
{
    delete m_desktopPreview;
}

void PositionTab::load()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
        configname = "kickerrc";
    else
        configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig c(configname, false, false);

    c.setGroup("General");

    // Magic numbers stolen from kdebase/kicker/core/global.cpp
    // PGlobal::sizeValue()
    int panelSize = c.readNumEntry("Size", 30);
    switch(panelSize) 
    {
        case 24: 
            m_sizeTiny->setChecked(true); 
            break;
        case 30: 
            m_sizeSmall->setChecked(true); 
            break;
        case 46: 
            m_sizeNormal->setChecked(true); 
            break;
        case 58: 
            m_sizeLarge->setChecked(true); 
            break;
        default: 
            m_sizeCustom->setChecked(true);
            m_customSlider->setValue(panelSize);
            m_customSpinbox->setValue(panelSize);
        break;
    }

    m_panelPos = c.readNumEntry("Position", PosBottom);
    m_panelAlign = c.readNumEntry("Alignment", QApplication::reverseLayout() ? AlignRight : AlignLeft);

    if (m_panelPos == PosTop)
    {
        if (m_panelAlign == AlignLeft)
            locationTopLeft->setOn(true);
        else if (m_panelAlign == AlignCenter)
            locationTop->setOn(true);
        else // if (m_panelAlign == AlignRight
            locationTopRight->setOn(true);
    }
    else if (m_panelPos == PosRight)
    {
        if (m_panelAlign == AlignLeft)
            locationRightTop->setOn(true);
        else if (m_panelAlign == AlignCenter)
            locationRight->setOn(true);
        else // if (m_panelAlign == AlignRight
            locationRightBottom->setOn(true);
    }
    else if (m_panelPos == PosBottom)
    {
        if (m_panelAlign == AlignLeft)
            locationBottomLeft->setOn(true);
        else if (m_panelAlign == AlignCenter)
            locationBottom->setOn(true);
        else // if (m_panelAlign == AlignRight
            locationBottomRight->setOn(true);
    }
    else // if (m_panelPos == PosLeft
    {
        if (m_panelAlign == AlignLeft)
            locationLeftTop->setOn(true);
        else if (m_panelAlign == AlignCenter)
            locationLeft->setOn(true);
        else // if (m_panelAlign == AlignRight
            locationLeftBottom->setOn(true);
    }
    int sizepercentage = c.readNumEntry( "SizePercentage", 100 );
    m_percentSlider->setValue( sizepercentage );
    m_percentSpinBox->setValue( sizepercentage );

    m_expandCheckBox->setChecked( c.readBoolEntry( "ExpandSize", true ) );
    
    lengthenPanel(sizepercentage);
    m_desktopPreview->setPreview(m_pretendDesktop->size());
    m_desktopPreview->start();
}

void PositionTab::save()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
        configname = "kickerrc";
    else
        configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig c(configname, false, false);

    c.setGroup("General");

    // Magic numbers stolen from kdebase/kicker/core/global.cpp
    // PGlobal::sizeValue()
    if (m_sizeTiny->isChecked())
    {
        c.writeEntry("Size",24);
    }
    else if (m_sizeSmall->isChecked())
    {
        c.writeEntry("Size",30);
    }
    else if (m_sizeNormal->isChecked())
    {
        c.writeEntry("Size",46);
    }
    else if (m_sizeLarge->isChecked())
    {
        c.writeEntry("Size",58);
    }
    else // if (m_sizeCustom->isChecked())
    {
        c.writeEntry("Size", m_customSlider->value());
    }

    c.writeEntry("Position", m_panelPos);
    c.writeEntry("Alignment", m_panelAlign);

    c.writeEntry( "SizePercentage", m_percentSlider->value() );
    c.writeEntry( "ExpandSize", m_expandCheckBox->isChecked() );
    c.sync();
}

void PositionTab::defaults()
{
    m_panelPos= PosBottom; // bottom of the screen
    m_percentSlider->setValue( 100 ); // use all space available
    m_percentSpinBox->setValue( 100 ); // use all space available
    m_expandCheckBox->setChecked( true ); // expand as required
    
    if (QApplication::reverseLayout())
    {
        // RTL lang aligns right
        m_panelAlign = AlignRight;
    }
    else
    {
        // everyone else aligns left
        m_panelAlign = AlignLeft;
    }

    m_sizeSmall->setChecked(true); // small size

    // update the magic drawing
    lengthenPanel(-1);
}

void PositionTab::movePanel(int whichButton)
{
    QPushButton* pushed = reinterpret_cast<QPushButton*>(m_locationGroup->find(whichButton));

    if (pushed == locationTopLeft)
    {
        m_panelAlign = AlignLeft;
        m_panelPos = PosTop;
    }
    else if (pushed == locationTop)
    {
        m_panelAlign = AlignCenter;
        m_panelPos = PosTop;
    }
    else if (pushed == locationTopRight)
    {
        m_panelAlign = AlignRight;
        m_panelPos = PosTop;
    }
    else if (pushed == locationLeftTop)
    {
        m_panelAlign = AlignLeft;
        m_panelPos = PosLeft;
    }
    else if (pushed == locationLeft)
    {
        m_panelAlign = AlignCenter;
        m_panelPos = PosLeft;
    }
    else if (pushed == locationLeftBottom)
    {
        m_panelAlign = AlignRight;
        m_panelPos = PosLeft;
    }
    else if (pushed == locationBottomLeft)
    {
        m_panelAlign = AlignLeft;
        m_panelPos = PosBottom;
    }
    else if (pushed == locationBottom)
    {
        m_panelAlign = AlignCenter;
        m_panelPos = PosBottom;
    }
    else if (pushed == locationBottomRight)
    {
        m_panelAlign = AlignRight;
        m_panelPos = PosBottom;
    }
    else if (pushed == locationRightTop)
    {
        m_panelAlign = AlignLeft;
        m_panelPos = PosRight;
    }
    else if (pushed == locationRight)
    {
        m_panelAlign = AlignCenter;
        m_panelPos = PosRight;
    }
    else if (pushed == locationRightBottom)
    {
        m_panelAlign = AlignRight;
        m_panelPos = PosRight;
    }

    lengthenPanel(-1);
}

void PositionTab::lengthenPanel(int sizePercent)
{
    if (sizePercent < 0)
    {
        sizePercent = m_percentSlider->value();
    }

    unsigned int x(0), y(0), x2(0), y2(0);
    unsigned int diff = 0;
    unsigned int panelSize = 4;
    
    if (m_sizeSmall->isChecked())
    {
        panelSize = panelSize * 3 / 2;
    }
    else if (m_sizeNormal->isChecked())
    {
        panelSize *= 2;
    }
    else if (m_sizeLarge->isChecked())
    {
        panelSize = panelSize * 5 / 2;
    }
    else if (m_sizeCustom->isChecked())
    {
        panelSize = panelSize * m_customSlider->value() / 24;
    }

    switch (m_panelPos)
    {
        case PosTop:
            x  = offsetX + margin;
            x2 = maxX - margin;
            y  = offsetY + margin;
            y2 = panelSize;
            
            diff =  x2 - ((x2 * sizePercent) / 100);
            if (m_panelAlign == AlignLeft)
            {
                x2  -= diff;
            }
            else if (m_panelAlign == AlignCenter)
            {
                x  += diff / 2;
                x2 -= diff;
            }
            else // m_panelAlign == AlignRight
            {
                x  += diff;
                x2 -= diff;
            }
            break;
        case PosLeft:
            x  = offsetX + margin;
            x2 = panelSize;
            y  = offsetY + margin;
            y2 = maxY - margin;
            
            diff =  y2 - ((y2 * sizePercent) / 100);
            if (m_panelAlign == AlignLeft)
            {
                y2  -= diff;
            }
            else if (m_panelAlign == AlignCenter)
            {
                y  += diff / 2;
                y2 -= diff;
            }
            else // m_panelAlign == AlignRight
            {
                y  += diff;
                y2 -= diff;
            }
            break;
        case PosBottom:
            x  = offsetX + margin;
            x2 = maxX - margin;
            y  = offsetY + maxY - panelSize;
            y2 = panelSize;
            
            diff =  x2 - ((x2 * sizePercent) / 100);
            if (m_panelAlign == AlignLeft)
            {
                x2  -= diff;
            }
            else if (m_panelAlign == AlignCenter)
            {
                x  += diff / 2;
                x2 -= diff;
            }
            else // m_panelAlign == AlignRight
            {
                x  += diff;
                x2 -= diff;
            }
            break;
        default: // case PosRight:
            x  = offsetX + maxX - panelSize;
            x2 = panelSize;
            y  = offsetY + margin;
            y2 = maxY - margin;
            
            diff =  y2 - ((y2 * sizePercent) / 100);
            if (m_panelAlign == AlignLeft)
            {
                y2  -= diff;
            }
            else if (m_panelAlign == AlignCenter)
            {
                y  += diff / 2;
                y2 -= diff;
            }
            else // m_panelAlign == AlignRight
            {
                y  += diff;
                y2 -= diff;
            }
            break;
    }

    m_pretendPanel->setGeometry(x, y, x2, y2);
}

void PositionTab::panelDimensionsChanged()
{
    lengthenPanel(-1);
}

void PositionTab::slotBGPreviewReady(int)
{
    KPixmap pm;
    if (QPixmap::defaultDepth() < 15)
    {
        pm.convertFromImage(*m_desktopPreview->image(), KPixmap::LowColor);
    }
    else
    {
        pm.convertFromImage(*m_desktopPreview->image());
    }

    m_pretendDesktop->setBackgroundPixmap(pm);
}



