/*
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
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

#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qpushbutton.h>

#include <kglobal.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <kdb/dbengine.h>
#include <kdb/plugin.h>
#include <kdb/plugininfodlg.h>

#include <pluginconfig.h>

#include <pluginconfig.moc>

using namespace KDB;

PluginConfig::PluginConfig( QWidget* parent, const char* name )
  : QWidget( parent, name )
{
  QGridLayout* layout = new QGridLayout( this, 4, 2, 15, 7 );

  m_pluginList = new QListBox( this );
  m_pluginList->insertStringList( DBENGINE->pluginNames() );
  connect( m_pluginList, SIGNAL( doubleClicked( QListBoxItem* ) ), this, SLOT( pluginSelected( QListBoxItem* ) ) );
  layout->addMultiCellWidget( m_pluginList, 0, 4, 0, 0 );

  QPushButton* btnInfo = new QPushButton( i18n( "Info..." ), this );
  layout->addWidget( btnInfo, 0, 1 );
  connect( btnInfo, SIGNAL( clicked() ), SLOT( slotInfo() ) );

  layout->setRowStretch( 3, 1 );
  layout->setColStretch( 0, 1 );
}

void PluginConfig::load()
{
}

void PluginConfig::save()
{
}

void PluginConfig::defaults()
{
}

void PluginConfig::slotInfo()
{
  int item = m_pluginList->currentItem();

  if( item >= 0 )
    pluginSelected( m_pluginList->item( item ) );
}

void PluginConfig::pluginSelected( QListBoxItem* item )
{
    Plugin* plugin = DBENGINE->findPlugin( item->text() );
    if (plugin) {
        PluginInfoDialog::showInfo( plugin, this );
    } else {
        qDebug( "Scheisse" ); // try i18n, could be interesting ;)
    }
}
