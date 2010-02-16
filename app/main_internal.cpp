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
#include <QSessionManager>

#include <shell/core.h>
#include <shell/mainwindow.h>
#include <shell/projectcontroller.h>
#include <shell/documentcontroller.h>
#include <shell/plugincontroller.h>
#include <shell/sessioncontroller.h>

#include "kdevideextension.h"
#include <KMessageBox>
#include <KProcess>

#include <iostream>
#include <QtCore/QTextStream>
#include <shell/session.h>

using KDevelop::Core;

class KDevelopApplication: public KApplication {
public:
    explicit KDevelopApplication(bool GUIenabled = true): KApplication(GUIenabled) {}

    void saveState( QSessionManager& sm ) {
        if (KDevelop::Core::self() && KDevelop::Core::self()->sessionController())
            sm.setRestartCommand(QStringList() << "kdevelop" << "-s" << KDevelop::Core::self()->sessionController()->activeSession()->id().toString());
        KApplication::saveState(sm);
    }

};

int main( int argc, char *argv[] )
{
static const char description[] = I18N_NOOP( "The KDevelop Integrated Development Environment" );
    KAboutData aboutData( "kdevelop", 0, ki18n( "KDevelop" ),
                          i18n("%1 (using KDevPlatform %2)", QString(VERSION), Core::version()).toUtf8(), ki18n(description), KAboutData::License_GPL,
                          ki18n( "Copyright 1999-2010, The KDevelop developers" ), KLocalizedString(), "http://www.kdevelop.org/" );

#include "shared_app_init.cpp"

    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    KDevelopApplication app;
    KDevIDEExtension::init();

    if(!getenv("KDEV_SESSION"))
    {
        QTextStream qerr(stderr);
        qerr << "Cannot run this internal app without KDEV_SESSION environment variable set!" << endl;
        exit(-127);
    }

    KSplashScreen* splash = 0;
    QString splashFile = KStandardDirs::locate( "appdata", "pics/kdevelop-splash.png" );
    if( !splashFile.isEmpty() )
    {
        QPixmap pm;
        pm.load( splashFile );
        splash = new KSplashScreen( pm );
        splash->show();
    }

    if(!Core::initialize(splash))
        return 5;
    
    KGlobal::locale()->insertCatalog( Core::self()->componentData().catalogName() );
    Core* core = Core::self();

    QStringList projectNames = args->getOptionList("project");
    if(!projectNames.isEmpty())
    {
        foreach(const QString& p, projectNames)
        {
            KUrl url(p);
            QString ext = QFileInfo( url.fileName() ).suffix();
            if( ext == "kdev4" )
            {
                core->projectController()->openProject( url );
            }
        }
    }

    int count=args->count();
    for(int i=0; i<count; ++i)
    {
        QString file=args->arg(i);
        //Allow opening specific lines in documents, like mydoc.cpp:10
        int lineNumberOffset = file.lastIndexOf(':');
        KTextEditor::Cursor line;
        if( lineNumberOffset != -1 )
        {
            bool isInt;
            int lineNr = file.mid(lineNumberOffset+1).toInt(&isInt);
            if (isInt)
            {
                file = file.left(lineNumberOffset);
                line = KTextEditor::Cursor(lineNr, 0);
            }
        }

        KUrl f(file);
        if( f.isRelative() )
            f=KUrl(QDir::currentPath(), file);

        if(!core->documentController()->openDocument(f, line))
            kWarning() << i18n("Could not open %1") << args->arg(i);
    }
    args->clear();

    return app.exec();
}

