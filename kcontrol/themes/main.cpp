/*
 * main.cpp
 *
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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

#include <kmessagebox.h>
#include <kimgio.h>

#define private public
#include <kcontrol.h>
#undef private

#include "themecreator.h"
#include "installer.h"
#include "global.h"
#include "options.h"
#include "about.h"
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstddirs.h>


ThemeCreator* theme = NULL;
static msg_handler oldMsgHandler = NULL;


//-----------------------------------------------------------------------------
class KThemesApplication : public KControlApplication
{
public:

  KThemesApplication(int &argc, char **arg, const char *name);
  ~KThemesApplication();

  virtual void init();
  virtual void apply();
  virtual void defaultValues();

protected:
  virtual void tweakUi(void);

private:
  Installer* mInstaller;
  Options* mOptions;
  About* mAbout;
};



//-----------------------------------------------------------------------------
KThemesApplication::KThemesApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  initMetaObject();
  init();

  mInstaller = NULL;
  theme = new ThemeCreator;

  if (runGUI())
  {
    tweakUi();

    addPage(mInstaller = new Installer(dialog), i18n("Installer"), "index-1.html" );
    addPage(mOptions = new Options(dialog), i18n("Contents"), "index-2.html" );
    addPage(mAbout = new About(dialog), i18n("About"), "index-3.html" );

    dialog->show();
  }
}


//-----------------------------------------------------------------------------
KThemesApplication::~KThemesApplication()
{
  if (theme) delete theme;
}


//-----------------------------------------------------------------------------
void KThemesApplication::tweakUi()
{
  KControlDialog* dlg = (KControlDialog*)getDialog();

  dlg->defaultBtn->setText(i18n("Extract"));
}


//-----------------------------------------------------------------------------
void KThemesApplication::init()
{
  //debug(i18n("No init necessary"));
    KGlobal::dirs()->addResourceType("themes", KStandardDirs::kde_default("data") + kapp->name() + "/Themes/");
}


//-----------------------------------------------------------------------------
void KThemesApplication::defaultValues()
{
  theme->extract();
}


//-----------------------------------------------------------------------------
void KThemesApplication::apply()
{
  mAbout->applySettings();
  mOptions->applySettings();
  mInstaller->applySettings();
  theme->install();
}


//=============================================================================
// Message handler
static void msgHandler(QtMsgType aType, const char* aMsg)
{
  QString appName = kapp->name();
  QString msg = aMsg;

  switch (aType)
  {
  case QtDebugMsg:
    kdebug(KDEBUG_INFO, 0, msg.ascii());
    break;

  case QtWarningMsg:
    fprintf(stderr, "%s: %s\n", appName.ascii(), msg.data());
    if (strncmp(aMsg,"KCharset:",9) != 0 &&
	strncmp(aMsg,"QGManager:",10) != 0 &&
	strncmp(aMsg,"QPainter:",9) != 0 &&
	strncmp(aMsg,"QPixmap:",8) != 0 &&
	strncmp(aMsg,"QFile:", 6) != 0)
    {
      KMessageBox::sorry(0, msg);
    }
    else kdebug(KDEBUG_INFO, 0, msg.ascii());
    break;

  case QtFatalMsg:
    fprintf(stderr, (appName+" "+i18n("fatal error")+": %s\n").ascii(), msg.ascii());
    KMessageBox::error(0, aMsg);
    abort();
  }
}


//-----------------------------------------------------------------------------
void init(void)
{
    oldMsgHandler = qInstallMsgHandler(msgHandler);

    if (!(Theme::mkdirhier(Theme::workDir().ascii()))) exit(1);
}


//-----------------------------------------------------------------------------
void cleanup(void)
{
    qInstallMsgHandler(oldMsgHandler);
}


//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  KThemesApplication app(argc, argv, "kthememgr");
  app.setTitle(i18n("Kde Theme Manager"));

  kimgioRegister();
//  init();

  if (app.runGUI()) app.exec();
  else app.init();

  cleanup();
  return 0;
}
