/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __Bgnd_h_Included__
#define __Bgnd_h_Included__

#include <qobject.h>
#include <qstring.h>
#include <qcolor.h>
#include <qmap.h>
#include <qevent.h>
#include <qwidget.h>

#include <kcmodule.h>
#include <bgdefaults.h>

class QCheckBox;
class QListBox;
class QComboBox;
class QStringList;
class QButtonGroup;
class QPalette;
class QLabel;
class QTabWidget;

class KColorButton;
class KBackgroundRenderer;
class KConfig;
class KStandardDirs;


/**
 * This class handles drops on the preview monitor.
 */
class KBGMonitor : public QWidget
{
    Q_OBJECT
public:

    KBGMonitor(QWidget *parent) : QWidget(parent) {
	setAcceptDrops(true);
    };

    // we don't want no steenking palette change
    virtual void setPalette(const QPalette &) {};

signals:
    void imageDropped(QString);

protected:
    virtual void dropEvent(QDropEvent *);
    virtual void dragEnterEvent(QDragEnterEvent *);
};


/**
 * The Desktop/Background tab in kcontrol.
 */
class KBackground: public KCModule
{
    Q_OBJECT

public:
    KBackground(QWidget *parent, const char *name);

    virtual void load();
    virtual void save();
    virtual void defaults();

signals:
    void changed(bool);

private slots:
    void slotSelectDesk(int desk);
    void slotCommonDesk(bool common);
    void slotBGMode(int mode);
    void slotBGSetup();
    void slotColor1(const QColor &);
    void slotColor2(const QColor &);
    void slotImageDropped(QString);
    void slotWPMode(int);
    void slotWallpaper(const QString &);
    void slotBrowseWallpaper();
    void slotSetupMulti();
    void slotPreviewDone(int);
    void slotMultiMode(bool);

private:
    void init();
    void apply();

    int m_Desk, m_Max;

    QCheckBox *m_pCBMulti;
    QComboBox *m_pBackgroundBox, *m_pWallpaperBox;
    QComboBox *m_pArrangementBox;
    QPushButton *m_pBGSetupBut, *m_pMSetupBut;
    QPushButton *m_pBrowseBut;
    QTabWidget *m_pTabWidget;
    QWidget *m_pTab1, *m_pTab2;
    QMap<QString,int> m_Wallpaper;

    KBackgroundRenderer *m_Renderer[_maxDesktops];
    KColorButton *m_pColor1But, *m_pColor2But;
    KBGMonitor *m_pMonitor;

    KStandardDirs *m_pDirs;
};


#endif // __Bgnd_h_Included__
