#include <fcntl.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "ksystraycmd.h"

#include <X11/Xlib.h>
#ifndef KDE_USE_FINAL
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#endif
#undef FocusOut
#undef FocusIn
#undef KeyPress
#undef KeyRelease


static KCmdLineOptions options[] =
{
  { "!+command", I18N_NOOP("Command to execute."), 0 },
  // "!" means: all options after command are treated as arguments to the command
  { "window <regexp>", I18N_NOOP("A regular expression matching the window title.\n"
                  "If you do not specify one, then the very first window\n"
				 "to appear will be taken. Not recommended!"), 0 },
  { "hidden", I18N_NOOP( "Hide the window to the tray on startup" ), 0 },
  { "startonshow", I18N_NOOP( "Wait until we're told to show the window before\n"
			      "executing the command" ), 0 },
  { "tooltip <text>", I18N_NOOP( "Sets the initial tooltip for the tray icon" ), 0 },
  { "keeprunning", I18N_NOOP( "Keep the tray icon even if the client exits. This option\n"
			 "has no effect unless startonshow is specified." ), 0 },
  { "quitonhide", I18N_NOOP( "Quit the client when we're told to hide the window\n"
             "has no effect unless startonshow is specified and implies keeprunning." ), 0 },
  /*  { "menuitem <item>", I18N_NOOP( "Adds a custom entry to the tray icon menu.\n"
      "The item should have the form text:command." ), 0 },*/
  { 0, 0, 0 }
};

int main( int argc, char *argv[] )
{
  KAboutData aboutData( "ksystraycmd", I18N_NOOP( "KSysTrayCmd" ),
			"KSysTrayCmd 0.1", 
			I18N_NOOP( "Allows any application to be kept in the system tray." ),
			KAboutData::License_GPL,
			"(C) 2001 Richard Moore (rich@kde.org)" );
  aboutData.addAuthor( "Richard Moore", 0, "rich@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;

  //
  // Setup the tray icon from the arguments.
  //
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  KSysTrayCmd cmd;

  // Read window title regexp
  QString title = args->getOption( "window" );
  if ( !title && (args->count() == 0) )
    KCmdLineArgs::usage(i18n("No command or window title specified"));
  cmd.setPattern( title );

  // Read the command
  QString command;
  for ( int i = 0; i < args->count(); i++ )
    command += QCString( args->arg(i) ) + " ";
  cmd.setCommand( command );

  // Tooltip
  QString tip = args->getOption( "tooltip" );
  if ( !tip.isEmpty() )
    cmd.setDefaultTip( tip );

  // Keep running flag
  if ( args->isSet( "keeprunning" )  )
    cmd.setNoQuit( true );

  if ( args->isSet( "quitonhide" ) ) {
    cmd.setNoQuit( true );
	cmd.setQuitOnHide( true );
  }

  // Start hidden
  if ( args->isSet( "hidden" ) )
    cmd.hideWindow();

  // Lazy invocation flag
  if ( args->isSet( "startonshow" ) ) {
    cmd.setStartOnShow( true );
    cmd.show();
  }
  else {
    if ( !cmd.start() )
      return 1;
  }

  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);
  args->clear();

  return app.exec();
}

