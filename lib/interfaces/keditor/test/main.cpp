/*
 * Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>
 */

#include "editortest.h"
#include <kapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char *description =
  I18N_NOOP("A KDE KPart Application");

static const char *version = "0.1";

static KCmdLineOptions options[] =
  {
    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    { 0, 0, 0 }
  };

int main(int argc, char **argv)
{
  KAboutData about("editortest", I18N_NOOP("EditorTest"), version, description,
                   KAboutData::License_GPL, "(C) 2001 Matthias Hölzer-Klüpfel", 0, 0, "mhk@caldera.de");
  about.addAuthor( "Matthias Hölzer-Klüpfel", 0, "mhk@caldera.de" );
  KCmdLineArgs::init(argc, argv, &about);
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app;

  // see if we are starting with session management
  if (app.isRestored())
    RESTORE(EditorTest)
    else
    {
      // no session.. just start up normally
      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

      if ( args->count() == 0 )
      {
        EditorTest *widget = new EditorTest;
        widget->show();
      }
      else
      {
        int i = 0;
        for (; i < args->count(); i++ )
        {
          EditorTest *widget = new EditorTest;
          widget->show();
          widget->load( args->url( i ) );
        }
      }
      args->clear();
    }

  return app.exec();
}
