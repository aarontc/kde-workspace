#ifndef __KFONT_VIEW_PART_H__
#define __KFONT_VIEW_PART_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFontViewPart
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 03/08/2002
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2002
////////////////////////////////////////////////////////////////////////////////

#include <kparts/part.h>

class CFontPreview;
class QPushButton;

class KFontViewPart : public KParts::ReadOnlyPart
{
    Q_OBJECT

    public:

    KFontViewPart(QWidget *parent=0, const char *name=0);
    virtual ~KFontViewPart();

    protected:

    virtual bool openFile();

    private slots:

    void installFont();

    private:

    CFontPreview *itsPreview;
    QPushButton  *itsInstallButton;
};

#endif
