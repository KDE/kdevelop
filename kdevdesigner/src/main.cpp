/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <qsplashscreen.h>

#include <kxmlguiclient.h>
#include "kdevdesigner.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kstandarddirs.h>

static const char description[] =
    I18N_NOOP("KDE GUI Designer");

static const char version[] = "0.2";

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open" ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("kdevdesigner", I18N_NOOP("KDevDesigner"), version, description,
                     KAboutData::License_GPL, "KDevDesigner Copyright: (C) 2004-2005 Alexander Dymo\nQt Designer Copyright: (C) 2000-2005 Trolltech AS All Rights Reserved", 0, 0);
    about.addAuthor( "Trolltech AS", "Qt Designer code (Free Edition)" );
    about.addAuthor( "Alexander Dymo", "Port to KDE, partification", "adymo@kdevelop.org" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;

    // see if we are starting with session management
    if (app.isRestored())
    {
        RESTORE(KDevDesigner);
    }
    else
    {
        // no session.. just start up normally
        QSplashScreen * splash = 0;
        QString splashFile = locate("data", "kdevelop/pics/kdevdesigner-splash.png");
        if (!splashFile.isNull())
        {
            QPixmap pm;
            pm.load(splashFile);
            splash = new QSplashScreen( pm );
            splash->show();
        }

        app.processEvents();

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if ( args->count() == 0 )
        {
            KDevDesigner *widget = new KDevDesigner;
            widget->show();
        }
        else
        {
            int i = 0;
            for (; i < args->count(); i++ )
            {
                KDevDesigner *widget = new KDevDesigner;
                widget->show();
                widget->load( args->url( i ) );
            }
        }
        args->clear();
        if (splash)
            delete splash;
    }

    return app.exec();
}
