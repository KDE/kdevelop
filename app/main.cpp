/***************************************************************************
 *   Copyright 2003-2009 Alexander Dymo <adymo@kdevelop.org>               *
 *   Copyright 2007 Ralf Habacker  <Ralf.Habacker@freenet.de>              *
 *   Copyright 2006-2007 Matt Rogers  <mattr@kde.org>                      *
 *   Copyright 2006-2007 Hamish Rodda <rodda@kde.org>                      *
 *   Copyright 2005-2007 Adam Treat <treat@kde.org>                        *
 *   Copyright 2003-2007 Jens Dagerbo <jens.dagerbo@swipnet.se>            *
 *   Copyright 2001-2002 Bernd Gehrmann <bernd@mail.berlios.de>            *
 *   Copyright 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>       *
 *   Copyright 2003 Roberto Raggi <roberto@kdevelop.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <config.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kxmlguiwindow.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <ksplashscreen.h>
#include <ktexteditor/cursor.h>

#include <QFileInfo>
#include <QPixmap>
#include <QTimer>
#include <QDir>

#include <shell/sessioncontroller.h>

#include <KMessageBox>
#include <KProcess>

#include <iostream>
#include <QtCore/QTextStream>

int main( int argc, char *argv[] )
{
static const char description[] = I18N_NOOP( "The KDevelop Integrated Development Environment" );
    KAboutData aboutData( "kdevelop", 0, ki18n( "KDevelop" ),
                          i18n("%1", QString(VERSION) ).toUtf8(), ki18n(description), KAboutData::License_GPL,
                          ki18n( "Copyright 1999-2009, The KDevelop developers" ), KLocalizedString(), "http://www.kdevelop.org" );
#include "shared_app_init.cpp"

    options.add("s <session>", ki18n("Session to load, use this also to start a new session by passing an unused name" ));
    options.add("sessions", ki18n( "List available sessions and quit" ));

    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    KApplication app;

    if(args->isSet("sessions"))
    {
        QTextStream qout(stdout);
        qout << endl << ki18n("Available sessions (use '-s HASH' to open a specific one):").toString() << endl << endl;
        qout << QString("%1").arg(ki18n("Hash").toString(), -38) << '\t' << ki18n("Session contents").toString() << endl;
        foreach(const KDevelop::SessionInfo& si, KDevelop::SessionController::availableSessionInfo())
        {
            qout << si.uuid.toString() << '\t' << si.description << endl;
        }
        return 0;
    }

    ///Manage sessions: There always needs a KDEV_SESSION to be set, so the duchain can be stored in the session-specific directory
    QString session = args->getOption("s");

    //No session is set, we have to pick one, then we restart kdevelop through kdev_starter, and forward all relevant arguments to it
    session = KDevelop::SessionController::defaultSessionId(session);

    //@todo Eventually show a session-picking dialog
    KProcess proc;
    proc << QFileInfo(QApplication::applicationFilePath()).path() + "/kdevelop.bin" ;
    //Forward all arguments, except -s as the internal app doesn't setup -s or --sessions arguments
    for(uint a = 1; a < argc; ++a) {
        if( qstrcmp( argv[a], "-s" ) == 0 ) {
            ++a;
        } else {
            proc << QString(argv[a]);
        }
    }
    proc.setEnv( "KDEV_SESSION", session );
    return proc.execute();
}

