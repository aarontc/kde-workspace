/*
    Copyright (C) 2004-2005 Oswald Buddenhagen <ossi@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KBACKEDCOMBOBOX_H
#define KBACKEDCOMBOBOX_H

#include <kcombobox.h>

class KBackedComboBox : public KComboBox {

  public:
	KBackedComboBox( QWidget *parent ) : KComboBox( false, parent ) {}
	void insertItem( const QString &id, const QString &name );
	void setCurrentId( const QString &id );
	const QString currentId() const;

  private:
	QMap<QString, QString> id2name, name2id;

};

#endif // KBACKEDCOMBOBOX_H
