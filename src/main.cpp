/***************************************************************************
 *   Copyright (C) 2003-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
 *   Copyright (C) 2007 by Ralf Habacker  <Ralf.Habacker@freenet.de>       *
 *   Copyright (C) 2006-2007 by Matt Rogers  <mattr@kde.org>               *
 *   Copyright (C) 2006-2007 by Hamish Rodda <rodda@kde.org>               *
 *   Copyright (C) 2005-2007 by Adam Treat <treat@kde.org>                 *
 *   Copyright (C) 2003-2007 by Jens Dagerbo <jens.dagerbo@swipnet.se>     *
 *   Copyright (C) 2001-2002 by Bernd Gehrmann <bernd@mail.berlios.de>     *
 *   Copyright (C) 2001-2002 by Matthias Hoelzer-Kluepfel <hoelzer@kde.org>*
 *   Copyright (C) 2003 by Roberto Raggi <roberto@kdevelop.org>            *
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

#include <QFileInfo>
#include <QPixmap>
#include <QTimer>

#include "core.h"
// #include "kdevconfig.h"
#include "mainwindow.h"
// #include "kdevenvironment.h"
// #include "kdevpartcontroller.h"
// #include "kdevlanguagecontroller.h"
#include "splashscreen.h"
#include "plugincontroller.h"
// #include "kdevprojectcontroller.h"
// #include "kdevdocumentcontroller.h"
// #include "kdevbackgroundparser.h"

#include "kdevideextension.h"

static KCmdLineOptions options[] =
    {
        { "profile <profile>", I18N_NOOP( "Profile to load" ), 0 },
        { "project <project>", I18N_NOOP( "Project to load" ), 0 },
        { "+file(s)", I18N_NOOP( "Files to load" ), 0 },
        KCmdLineLastOption // End of options.
    };

int main( int argc, char *argv[] )
{
    static const char description[] = I18N_NOOP( "The KDevelop Integrated Development Environment" );
    KAboutData aboutData( "kdevelop", I18N_NOOP( "KDevelop" ),
                          VERSION, description, KAboutData::License_GPL,
                          I18N_NOOP( "(c) 1999-2007, The KDevelop developers" ), "", "http://www.kdevelop.org" );
    aboutData.addAuthor( "Matt Rogers", I18N_NOOP( "Maintainer" ), "mattr@kde.org");
    aboutData.addAuthor( "Alexander Dymo", I18N_NOOP( "Co-maintainer, architecture, Sublime UI, Ruby support" ), "adymo@kdevelop.org" );
    aboutData.addAuthor( "Amilcar do Carmo Lucas", I18N_NOOP( "Release coordinator, API documentation, Doxygen and autoproject patches" ), "amilcar@ida.ing.tu-bs.de" );
    aboutData.addAuthor( "Bernd Gehrmann", I18N_NOOP( "Initial idea, basic architecture, much initial source code" ), "bernd@kdevelop.org" );
    aboutData.addAuthor( "Caleb Tennis", I18N_NOOP( "KTabBar, bugfixes" ), "caleb@aei-tech.com" );
    aboutData.addAuthor( "Richard Dale", I18N_NOOP( "Java & Objective C support" ), "Richard_Dale@tipitina.demon.co.uk" );
    aboutData.addAuthor( "John Birch", I18N_NOOP( "Debugger frontend" ), "jbb@kdevelop.org" );
    aboutData.addAuthor( "Sandy Meier", I18N_NOOP( "PHP support, context menu stuff" ), "smeier@kdevelop.org" );
    aboutData.addAuthor( "Kurt Granroth", I18N_NOOP( "KDE application templates" ), "kurth@granroth.org" );
    aboutData.addAuthor( "Ian Reinhart Geiser", I18N_NOOP( "Dist part, bash support, application templates" ), "geiseri@yahoo.com" );
    aboutData.addAuthor( "Matthias Hoelzer-Kluepfel", I18N_NOOP( "Several components, htdig indexing" ), "hoelzer@kde.org" );
    aboutData.addAuthor( "Victor Roeder", I18N_NOOP( "Help with Automake manager and persistant class store" ), "victor_roeder@gmx.de" );
    aboutData.addAuthor( "Harald Fernengel", I18N_NOOP( "Ported to Qt 3, patches, valgrind, diff and perforce support" ), "harry@kdevelop.org" );
    aboutData.addAuthor( "Roberto Raggi", I18N_NOOP( "QEditor component, code completion, Abbrev component, C++ support, Java support" ), "roberto@kdevelop.org" );
    aboutData.addAuthor( "Hamish Rodda", I18N_NOOP( "Text editor integration, definition-use chain" ), "rodda@kde.org" );
    aboutData.addAuthor( "Simon Hausmann", I18N_NOOP( "Help with KParts infrastructure" ), "hausmann@kde.org" );
    aboutData.addAuthor( "Oliver Kellogg", I18N_NOOP( "Ada support" ), "okellogg@users.sourceforge.net" );
    aboutData.addAuthor( "Jakob Simon-Gaarde", I18N_NOOP( "QMake projectmanager" ), "jsgaarde@tdcspace.dk" );
    aboutData.addAuthor( "Andreas Pakulat", I18N_NOOP( "Project Management Support, QMake Projectmanager" ), "apaku@gmx.de" );
    aboutData.addAuthor( "F@lk Brettschneider", I18N_NOOP( "MDI modes, QEditor, bugfixes" ), "falkbr@kdevelop.org" );
    aboutData.addAuthor( "Mario Scalas", I18N_NOOP( "PartExplorer, redesign of CvsPart, patches, bugs(fixes)" ), "mario.scalas@libero.it" );
    aboutData.addAuthor( "Jens Dagerbo", I18N_NOOP( "Replace, Bookmarks, FileList and CTags2 plugins. Overall improvements and patches" ), "jens.dagerbo@swipnet.se" );
    aboutData.addAuthor( "Julian Rockey", I18N_NOOP( "Filecreate part and other bits and patches" ), "linux@jrockey.com" );
    aboutData.addAuthor( "Dukju Ahn", I18N_NOOP( "Subversion plugin, Custom Make Manager, Overall improvements" ), "dukjuahn@gmail.com" );
    aboutData.addCredit( "Ajay Guleria", I18N_NOOP( "ClearCase support" ), "ajay_guleria@yahoo.com" );
    aboutData.addCredit( "Marek Janukowicz", I18N_NOOP( "Ruby support" ), "child@t17.ds.pwr.wroc.pl" );

    aboutData.addCredit( "The KWrite authors", I18N_NOOP( "Kate editor component" ), "kwrite-devel@kde.org" );
    aboutData.addCredit( "The KHTML authors", I18N_NOOP( "HTML documentation component" ), "kwrite-devel@kde.org" );
    aboutData.addCredit( "Robert Moniot", I18N_NOOP( "Fortran documentation" ), "moniot@fordham.edu" );
    aboutData.addCredit( "Ka-Ping Yee", I18N_NOOP( "Python documentation utility" ), "ping@lfw.org" );
    aboutData.addCredit( "Dimitri van Heesch", I18N_NOOP( "Doxygen wizard" ), "dimitri@stack.nl" );
    aboutData.addCredit( "Hugo Varotto", I18N_NOOP( "Fileselector component" ), "hugo@varotto-usa.com" );
    aboutData.addCredit( "Matt Newell", I18N_NOOP( "Fileselector component" ), "newellm@proaxis.com" );
    aboutData.addCredit( "Trolltech AS", I18N_NOOP( "Designer code" ), "info@trolltech.com" );
    aboutData.addCredit( "Daniel Engelschalt", I18N_NOOP( "C++ code completion, persistant class store" ), "daniel.engelschalt@gmx.net" );
    aboutData.addCredit( "Stephane ANCELOT", I18N_NOOP( "Patches" ), "sancelot@free.fr" );
    aboutData.addCredit( "Jens Zurheide", I18N_NOOP( "Patches" ), "jens.zurheide@gmx.de" );
    aboutData.addCredit( "Luc Willems", I18N_NOOP( "Help with Perl support" ), "Willems.luc@pandora.be" );
    aboutData.addCredit( "Marcel Turino", I18N_NOOP( "Documentation index view" ), "M.Turino@gmx.de" );
    aboutData.addCredit( "Yann Hodique", I18N_NOOP( "Patches" ), "Yann.Hodique@lifl.fr" );
    aboutData.addCredit( "Tobias Gl\303\244\303\237er" , I18N_NOOP( "Documentation Finder,  qmake projectmanager patches, usability improvements, bugfixes ... " ), "tobi.web@gmx.de" );
    aboutData.addCredit( "Andreas Koepfle" , I18N_NOOP( "QMake project manager patches" ), "koepfle@ti.uni-mannheim.de" );
    aboutData.addCredit( "Sascha Cunz" , I18N_NOOP( "Cleanup and bugfixes for qEditor, AutoMake and much other stuff" ), "mail@sacu.de" );
    aboutData.addCredit( "Robert Gruber" , I18N_NOOP( "SnippetPart, debugger and usability patches" ), "rgruber@users.sourceforge.net" );
    aboutData.addCredit( "Zoran Karavla", I18N_NOOP( "Artwork for the ruby language" ), "webmaster@the-error.net", "http://the-error.net" );

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    Q_UNUSED(args)
    KApplication app;

    KDevIDEExtension::init();

    SplashScreen *splash = 0;
    QString splashFile = KStandardDirs::locate( "appdata", "pics/kdevelop-splash.png" );
    if ( !splashFile.isEmpty() )
    {
        QPixmap pm;
        pm.load( splashFile );
        splash = new SplashScreen( pm );
        splash->show();
        splash->repaint();
    }

    using namespace KDevelop;

    //initialize the api object
    //WARNING! the order is important
    Core::initialize();
/*    Core::setPartController( new PartController );
    Core::setDocumentController( new DocumentController );*/
    Core::self()->pluginController()->loadPlugins( PluginController::Global );
//     Core::setLanguageController( new LanguageController );
//     Core::setProjectController( new ProjectController );
//     Core::setBackgroundParser( new BackgroundParser );
//     Core::setEnvironment( new Environment );

    if ( splash )
    {
        QObject::connect(Core::self()->pluginController(), SIGNAL(loadingPlugin(const QString&)),
                         splash, SLOT(showMessage(const QString&)));
        QTimer::singleShot(0, splash, SLOT(deleteLater()));
//         QObject::connect( Core::documentController(),
//                           SIGNAL( openingDocument( const QString & ) ),
//                           splash, SLOT( showMessage( const QString & ) ) );

        splash->showMessage( i18n( "Starting GUI" ) );
    }

//     bool openProject = false;
//     QByteArray projectName = args->getOption("project");
//     if ( !projectName.isEmpty() )
//     {
//         Core::projectController()->openProject( KUrl(projectName) );
//         openProject = true;
//     }
//     else if( args->count() > 0 )
//     {
//         KUrl url = args->url( 0 );
//         QString ext = QFileInfo( url.fileName() ).suffix();
//         if( ext == "kdev4" )
//         {
//             Core::projectController()->openProject( url );
//             openProject = true;
//         }
//     }
//
//     if( !openProject )
//     {
//         for( int a=0; a<args->count(); ++a )
//         {
//             Core::documentController()->editDocument( KUrl( args->url( a ) ) );
//         }
//     }

    return app.exec();
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
