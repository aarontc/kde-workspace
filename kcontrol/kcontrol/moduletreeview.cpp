/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 
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
 
*/                                                                            

#include <qheader.h>
#include <qstring.h>

#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kdesktopfile.h>

#include "moduletreeview.h"
#include "modules.h"
#include "global.h"

ModuleTreeView::ModuleTreeView(ConfigModuleList *list, QWidget * parent, const char * name)
  : KListView(parent, name)
  , _modules(list)
{
  setFrameStyle(QFrame::WinPanel | QFrame::Sunken);   
  addColumn("");   
  setAllColumnsShowFocus(true);
  header()->hide();

  connect(this, SIGNAL(currentChanged(QListViewItem*)), 
		  this, SLOT(slotItemSelected(QListViewItem*)));
}

void ModuleTreeView::fill()
{
  clear();
  
  QString rootlabel;
  
  if (KCGlobal::system())
	rootlabel = i18n("Settings for system %1").arg(KCGlobal::hostName());
  else
	rootlabel = i18n("Settings for user %1").arg(KCGlobal::userName());
  
  // add the root node
  ModuleTreeItem* root = new ModuleTreeItem(this, rootlabel);
  root->setPixmap(0, KGlobal::iconLoader()->loadIcon("kcontrol", KIconLoader::Small));
  
  ConfigModule *module;
  for (module=_modules->first(); module != 0; module=_modules->next())
    {
      if (module->library().isEmpty())
		continue;
      
      if (KCGlobal::system()) {
		if (!module->onlyRoot())
		  continue;
	  }
      else {
		if (module->onlyRoot() && !KCGlobal::root())
		  continue;
	  }
      
      ModuleTreeItem *parent;
      parent = root;
      parent = getGroupItem(parent, module->groups());
      new ModuleTreeItem(parent, module);
    }
  
  setOpen(root, true);
  setMinimumWidth(columnWidth(0)+22);
}

void ModuleTreeView::makeSelected(ConfigModule *module)
{
  ModuleTreeItem *item = static_cast<ModuleTreeItem*>(firstChild());
 
  updateItem(item, module);
}

void ModuleTreeView::updateItem(ModuleTreeItem *item, ConfigModule *module)
{
  while (item)
    {
	  if (item->childCount() != 0)
		updateItem(static_cast<ModuleTreeItem*>(item->firstChild()), module);
	  if (item->module() == module)
		{
		  setSelected(item, true);
		  break;
		}
	  item = static_cast<ModuleTreeItem*>(item->nextSibling());
    }
}

void ModuleTreeView::makeVisible(ConfigModule *module)
{
  ModuleTreeItem *item;
  
  item = static_cast<ModuleTreeItem*>(firstChild());
  setOpen(item, true);

  QStringList::ConstIterator it;
  for (it=module->groups().begin(); it != module->groups().end(); it++)
    {
      item = static_cast<ModuleTreeItem*>(item->firstChild());
      while (item)
		{
		  if (item->tag() == *it)
			{
			  setOpen(item, true);
			  break;
			}
		  
		  item = static_cast<ModuleTreeItem*>(item->nextSibling());
		}
    }
  
  // make the item visible
  if (item)
    ensureItemVisible(item);
}

ModuleTreeItem *ModuleTreeView::getGroupItem(ModuleTreeItem *parent, const QStringList& groups)
{
  QString path;
  
  ModuleTreeItem *item = parent;
  
  QStringList::ConstIterator it;
  for (it=groups.begin(); it != groups.end(); it++)
    {
      path += *it + "/";
	  
      parent = item;
      item = static_cast<ModuleTreeItem*>(item->firstChild());

      while (item)
		{
		  if (static_cast<ModuleTreeItem*>(item)->tag() == *it)
			break;
		  
		  item = static_cast<ModuleTreeItem*>(item->nextSibling());
		}

      if (!item)
		{
		  // create new branch
		  ModuleTreeItem *iitem = new ModuleTreeItem(parent);
		  iitem->setTag(*it);
		  
		  // now decorate the branch
		  KDesktopFile directory(locate("apps", "Settings/"+path+".directory"));
		  iitem->setText(0, directory.readEntry("Name", *it));
		  QPixmap icon = KGlobal::iconLoader()->loadIcon(directory.readEntry("Icon"),
														 KIconLoader::Small, 0, true);
		  if(icon.isNull())
			icon = KGlobal::iconLoader()->loadIcon("package.png", KIconLoader::Small);

		  iitem->setPixmap(0, icon);
		  
		  return iitem;
		}
    }
  return item;
}

void ModuleTreeView::slotItemSelected(QListViewItem* item)
{
  if (static_cast<ModuleTreeItem*>(item)->module())
	emit moduleSelected(static_cast<ModuleTreeItem*>(item)->module());
}

ModuleTreeItem::ModuleTreeItem(QListViewItem *parent, ConfigModule *module)
  : QListViewItem(parent)
  , _module(module)
  , _tag(QString::null)
{
  if (_module)
	{
	  setText(0, module->name());
	  setPixmap(0, module->icon());
	}
}

ModuleTreeItem::ModuleTreeItem(QListView *parent, ConfigModule *module)
  : QListViewItem(parent)
  , _module(module)
  , _tag(QString::null)
{
  if (_module)
	{
	  setText(0, module->name());
	  setPixmap(0, module->icon());
	}
}

ModuleTreeItem::ModuleTreeItem(QListViewItem *parent, const QString& text)
  : QListViewItem(parent, text)
  , _module(0)
  , _tag(QString::null) {}

ModuleTreeItem::ModuleTreeItem(QListView *parent, const QString& text)
  : QListViewItem(parent, text)
  , _module(0)
  , _tag(QString::null) {}
