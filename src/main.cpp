/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <kaboutdata.h>
#include <kapp.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "toplevel.h"
#include "core.h"
#include "config.h"

#if 0	
static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
};
#endif


int main(int argc, char *argv[])
{
    static const char *description = I18N_NOOP("Gideon");

    KAboutData aboutData("gideon", I18N_NOOP("Gideon"),
                         VERSION, description, KAboutData::License_GPL,
                         "(c) 1999-2001 Bernd Gehrmann & others");
    aboutData.addAuthor("Bernd Gehrmann", "Everything not credited elsewhere :-)", "bernd@kdevelop.org");
    aboutData.addAuthor("Richard Dale", "Java & Objective C support", "Richard_Dale@tipitina.demon.co.uk");
    aboutData.addAuthor("Jonas Nordin", "C++ source parser", "jonas.nordin@syncom.se");
    aboutData.addAuthor("John Birch", "Debugger frontend", "jbb@kdevelop.org");
    aboutData.addAuthor("Sandy Meier", "PHP support, context menu stuff", "smeier@kdevelop.org");
    aboutData.addAuthor("Kurt Granroth", "KDE application templates", "kurth@granroth.org");
    aboutData.addAuthor("Ian Reinhart Geiser", "Application templates", "geiseri@yahoo.com");
    aboutData.addAuthor("Matthias Hoelzer-Kluepfel", "Several parts, htdig indexing", "hoelzer@kde.org");
    aboutData.addAuthor("The KWrite authors", "editor component", "kwrite-devel@kde.org");
    aboutData.addAuthor("Robert Moniot", "ftnchek documentation", "moniot@fordham.edu");
    aboutData.addAuthor("Ka-Ping Yee", "pydoc utility", "ping@lfw.org");
    aboutData.addAuthor("Dimitri van Heesch", "doxywizard", "dimitri@stack.nl");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;
    Core core;
    
    return app.exec();
}
