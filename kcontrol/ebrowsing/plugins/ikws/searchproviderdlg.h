/*
 * Copyright (c) 2000 Malte Starostik <malte@kde.org>
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


#ifndef __SEARCHPROVIDERDLG_H___
#define __SEARCHPROVIDERDLG_H___

#include <kdialogbase.h>

class KLineEdit;
class KComboBox;
class SearchProvider;

class SearchProviderDialog : public KDialogBase {
    Q_OBJECT

public:
    SearchProviderDialog(SearchProvider *provider, QWidget *parent = 0, const char *name = 0);

    SearchProvider *provider() { return m_provider; }

protected slots:
    void slotChanged();
    void slotOk();

private:
    SearchProvider *m_provider;

    KLineEdit *m_name;
    KLineEdit *m_query;
    KLineEdit *m_keys;
    KComboBox *m_charset;
};

#endif
