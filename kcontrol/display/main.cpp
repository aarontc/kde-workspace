/*
  main.cpp 

  written 1997-1999  by Mark Donohoe, Martin Jones, Matthias Hoelzer, Matthias Ettrich

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


#include <stdio.h>

#include <kcontrol.h>
#include <kimgio.h>
#include "display.h"
#include "colorscm.h"
#include "scrnsave.h"
#include "general.h"
#include "backgnd.h"
#include "fonts.h"
#include "energy.h"
#include "advanced.h"
#include <qfont.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kprocess.h>
#include <X11/Xlib.h>


bool runResourceManager = FALSE;

class KDisplayApplication : public KControlApplication
{
public:

  KDisplayApplication(int &argc, char **arg, const char *name);

  void init();
  void apply();
  void defaultValues();
  void writeQDesktopProperties( QPalette pal, QFont font);

private:

  KColorScheme *colors;
  KScreenSaver *screensaver;
  KFonts *fonts;
  KGeneral *general;
  KBackground *background;
  KEnergy *energy;
  KAdvanced *advanced;
};


KDisplayApplication::KDisplayApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  colors = 0; screensaver = 0; fonts = 0; general = 0; background = 0;
  energy = 0; advanced = 0;

  if (runGUI())
    {
      if (!pages || pages->contains("background"))
        addPage(background = new KBackground(dialog, KDisplayModule::Setup),
		i18n("&Background"), "kdisplay-3.html");
      if (!pages || pages->contains("screensaver"))
	addPage(screensaver = new KScreenSaver(dialog, KDisplayModule::Setup),
		i18n("&Screensaver"), "kdisplay-4.html");

      if (!pages || pages->contains("colors"))
	addPage(colors = new KColorScheme(dialog, KDisplayModule::Setup),
		i18n("&Colors"), "kdisplay-5.html");
      if (!pages || pages->contains("fonts"))
	addPage(fonts = new KFonts(dialog, KDisplayModule::Setup),
		i18n("&Fonts"), "kdisplay-6.html");
      if (!pages || pages->contains("style"))
	addPage(general = new KGeneral(dialog, KDisplayModule::Setup),
		i18n("&Style"), "kdisplay-7.html");
      if (!pages || pages->contains("energy"))
	addPage(energy = new KEnergy(dialog, KDisplayModule::Setup),
		i18n("&Energy"), "kdisplay-8.html");
      if (!pages || pages->contains("advanced"))
	addPage(advanced = new KAdvanced(dialog, KDisplayModule::Setup),
		i18n("&Advanced"), "kdisplay-8.html");

      if (background || screensaver || colors || fonts || general || energy
          || advanced)
        dialog->show();
      else
        {
          fprintf(stderr, i18n("usage: kcmdisplay [-init | {background,screensaver,"
		"colors,fonts,style,energy,advanced}]\n").ascii());
          justInit = TRUE;
        }

    }
}


void KDisplayApplication::init()
{
  KColorScheme *colors = new KColorScheme(0, KDisplayModule::Init);
  delete colors;
  KBackground *background =  new KBackground(0, KDisplayModule::Init);
  delete background;
  KScreenSaver *screensaver = new KScreenSaver(0, KDisplayModule::Init);
  delete screensaver;
  KEnergy *energy = new KEnergy(0, KDisplayModule::Init);
  delete energy;
  KFonts *fonts = new KFonts(0, KDisplayModule::Init);
  delete fonts;
  KAdvanced *advanced = new KAdvanced(0, KDisplayModule::Init);
  
  writeQDesktopProperties( colors->createPalette(), KGlobal::generalFont() );
  
  KGeneral *general = new KGeneral(0, KDisplayModule::Init);
  delete general;

  KConfigGroupSaver saver(KGlobal::config(), "X11");
  if (KGlobal::config()->readBoolEntry( "useResourceManager", true )){
      KProcess proc;
      proc.setExecutable("krdb");
      proc.start( KProcess::Block );
  }


}


void KDisplayApplication::apply()
{
  if (colors)
    colors->applySettings();
  if (background)
    background->applySettings();
  if (screensaver)
    screensaver->applySettings();
  if (energy)
    energy->applySettings();
  if (fonts)
    fonts->applySettings();
  if (general)
    general->applySettings();
  if (advanced)
    advanced->applySettings();

  kapp->config()->sync();
  
  if (colors || fonts) {
      QPalette pal = colors?colors->createPalette():qApp->palette();

      KConfig *config = KGlobal::config();
      config->reparseConfiguration();
      config->setGroup( "General" );
      QFont font = KGlobal::generalFont();
      font = config->readFontEntry( "font", &font);
      writeQDesktopProperties( pal, font);
  }

  
  
  if ( runResourceManager ) {
      QApplication::setOverrideCursor( waitCursor );
      KProcess proc;
      proc.setExecutable("krdb");
      proc.start( KProcess::Block );
      QApplication::restoreOverrideCursor();
      runResourceManager = FALSE;
  }
}

void KDisplayApplication::defaultValues()
{
  if (colors)
    colors->defaultSettings();
  if (background)
    background->defaultSettings();
  if (screensaver)
    screensaver->defaultSettings();
  if (energy)
    energy->defaultSettings();
  if (fonts)
    fonts->defaultSettings();
  if (general)
    general->defaultSettings();
  if (advanced)
    advanced->defaultSettings();
}

void KDisplayApplication::writeQDesktopProperties( QPalette pal, QFont font)
{

    QByteArray properties;
    QDataStream d( properties, IO_WriteOnly );

    d << pal << font;

    Atom a = XInternAtom(qt_xdisplay(), "_QT_DESKTOP_PROPERTIES", FALSE );

    XChangeProperty(qt_xdisplay(),  qt_xrootwin(),
		    a, a, 8, PropModeReplace,
		    (unsigned char*) properties.data(), properties.size());
    QApplication::flushX();
}

int main(int argc, char **argv)
{
  //QApplication::setColorSpec( QApplication::ManyColor );
  // Please don't use this as it prevents users from choosing exactly the
  // colors they want - Mark Donohoe

  KDisplayApplication app(argc, argv, "kcmdisplay");
  app.setTitle(i18n("Display settings"));

  kimgioRegister();

  if (app.runGUI())
    return app.exec();
  else
    {
      app.init();
      return 0;
    }
}




