    /*

    Greeter module for xdm
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000 Oswald Buddenhagen <ossi@kde.org>


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
 

#ifndef KGREETER_H
#define KGREETER_H

# include "kdm-config.h"

#include <qglobal.h>

#define WMRC ".wmrc"

#include <qlineedit.h>
#include <qframe.h>
#undef index
#include <qiconview.h>

class QTimer;
class QIconView;
class QLabel;
class QPushButton;
class QComboBox;

#include <kpassdlg.h>

#include "kfdialog.h"
#include "kdmshutdown.h"

class KdmClock;

class KLoginLineEdit : public QLineEdit {
     Q_OBJECT

public:
     KLoginLineEdit( QWidget *parent = 0) : QLineEdit(parent) {}

signals:
     void lost_focus();

protected:
     void focusOutEvent( QFocusEvent *e);
};

class KGreeter : public QFrame {
     Q_OBJECT

public:
    KGreeter(QWidget *parent, const char *t);
    void ReturnPressed();
    void SetTimer();

public slots:
     void go_button_clicked();
     void cancel_button_clicked();
     void quit_button_clicked();
     void shutdown_button_clicked();
     void timerDone();
     void slot_user_name( QIconViewItem*);
     bool restrict();
     bool restrict_nologin();
     bool restrict_expired();
     bool restrict_nohome();
     bool restrict_time();
     void load_wm();
     void save_wm();

protected:
     void timerEvent( QTimerEvent * ) {};

private:
     QTimer*        timer;
     QIconView*     user_view;
     KdmClock*      clock;
     QLabel*        pixLabel;
     QLabel*        loginLabel;
     QLabel*        sessionargLabel;
     QLabel*        passwdLabel;
     QLabel*        failedLabel;
     KLoginLineEdit*     loginEdit;
     KPasswordEdit*     passwdEdit; 
     QFrame*        separator;
     QPushButton*   goButton;
     QPushButton*   chooserButton;
     QPushButton*   quitButton;
     QPushButton*   cancelButton;
     QPushButton*   shutdownButton;
     QComboBox*     sessionargBox;

//     struct passwd *pwd;

//#ifdef USESHADOW
//     struct spwd *swd;
//#endif

    void	insertUsers( QIconView *);

};



#endif /* KGREETER_H */

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */
