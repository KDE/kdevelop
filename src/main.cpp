/***************************************************************************
 *   Copyright 2003-2007 Alexander Dymo  <adymo@kdevelop.org>              *
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
#include <ktexteditor/cursor.h>

#include <QFileInfo>
#include <QPixmap>
#include <QTimer>
#include <QDir>

#include <shell/core.h>
#include <shell/mainwindow.h>
#include <shell/projectcontroller.h>
#include <shell/documentcontroller.h>
#include <shell/plugincontroller.h>

#include "splashscreen.h"
#include "kdevideextension.h"

using KDevelop::Core;

int main( int argc, char *argv[] )
{
    static const char description[] = I18N_NOOP( "The KDevelop Integrated Development Environment" );
    KAboutData aboutData( "kdevelop", 0, ki18n( "KDevelop" ),
                          VERSION, ki18n(description), KAboutData::License_GPL,
                          ki18n( "(c) 1999-2009, The KDevelop developers" ), KLocalizedString(), "http://www.kdevelop.org" );
    aboutData.addAuthor( ki18n("Andreas Pakulat"), ki18n( "Maintainer, Architecture, VCS Support, Project Management Support, QMake Projectmanager" ), "apaku@gmx.de" );
    aboutData.addAuthor( ki18n("Matt Rogers"), KLocalizedString(), "mattr@kde.org");
    aboutData.addAuthor( ki18n("Alexander Dymo"), ki18n( "Architecture, Sublime UI, Ruby support" ), "adymo@kdevelop.org" );
    aboutData.addAuthor( ki18n("Amilcar do Carmo Lucas"), ki18n( "Release coordinator, API documentation, Doxygen and autoproject patches" ), "amilcar@kdevelop.org" );
    aboutData.addAuthor( ki18n("Bernd Gehrmann"), ki18n( "Initial idea, basic architecture, much initial source code" ), "bernd@kdevelop.org" );
    aboutData.addAuthor( ki18n("Caleb Tennis"), ki18n( "KTabBar, bugfixes" ), "caleb@aei-tech.com" );
    aboutData.addAuthor( ki18n("Richard Dale"), ki18n( "Java & Objective C support" ), "Richard_Dale@tipitina.demon.co.uk" );
    aboutData.addAuthor( ki18n("John Birch"), ki18n( "Debugger frontend" ), "jbb@kdevelop.org" );
    aboutData.addAuthor( ki18n("Sandy Meier"), ki18n( "PHP support, context menu stuff" ), "smeier@kdevelop.org" );
    aboutData.addAuthor( ki18n("Kurt Granroth"), ki18n( "KDE application templates" ), "kurth@granroth.org" );
    aboutData.addAuthor( ki18n("Ian Reinhart Geiser"), ki18n( "Dist part, bash support, application templates" ), "geiseri@yahoo.com" );
    aboutData.addAuthor( ki18n("Matthias Hoelzer-Kluepfel"), ki18n( "Several components, htdig indexing" ), "hoelzer@kde.org" );
    aboutData.addAuthor( ki18n("Victor Roeder"), ki18n( "Help with Automake manager and persistant class store" ), "victor_roeder@gmx.de" );
    aboutData.addAuthor( ki18n("Harald Fernengel"), ki18n( "Ported to Qt 3, patches, valgrind, diff and perforce support" ), "harry@kdevelop.org" );
    aboutData.addAuthor( ki18n("Roberto Raggi"), ki18n( "QEditor component, code completion, Abbrev component, C++ support, Java support" ), "roberto@kdevelop.org" );
    aboutData.addAuthor( ki18n("Hamish Rodda"), ki18n( "Text editor integration, definition-use chain" ), "rodda@kde.org" );
    aboutData.addAuthor( ki18n("Simon Hausmann"), ki18n( "Help with KParts infrastructure" ), "hausmann@kde.org" );
    aboutData.addAuthor( ki18n("Oliver Kellogg"), ki18n( "Ada support" ), "okellogg@users.sourceforge.net" );
    aboutData.addAuthor( ki18n("Jakob Simon-Gaarde"), ki18n( "QMake projectmanager" ), "jsgaarde@tdcspace.dk" );
    aboutData.addAuthor( ki18n("Falk Brettschneider"), ki18n( "MDI modes, QEditor, bugfixes" ), "falkbr@kdevelop.org" );
    aboutData.addAuthor( ki18n("Mario Scalas"), ki18n( "PartExplorer, redesign of CvsPart, patches, bugs(fixes)" ), "mario.scalas@libero.it" );
    aboutData.addAuthor( ki18n("Jens Dagerbo"), ki18n( "Replace, Bookmarks, FileList and CTags2 plugins. Overall improvements and patches" ), "jens.dagerbo@swipnet.se" );
    aboutData.addAuthor( ki18n("Julian Rockey"), ki18n( "Filecreate part and other bits and patches" ), "linux@jrockey.com" );
    aboutData.addAuthor( ki18n("Dukju Ahn"), ki18n( "Subversion plugin, Custom Make Manager, Overall improvements" ), "dukjuahn@gmail.com" );
    aboutData.addCredit( ki18n("Ajay Guleria"), ki18n( "ClearCase support" ), "ajay_guleria@yahoo.com" );
    aboutData.addCredit( ki18n("Marek Janukowicz"), ki18n( "Ruby support" ), "child@t17.ds.pwr.wroc.pl" );

    aboutData.addCredit( ki18n("The KWrite authors"), ki18n( "Kate editor component" ), "kwrite-devel@kde.org" );
    aboutData.addCredit( ki18n("The KHTML authors"), ki18n( "HTML documentation component" ), "kwrite-devel@kde.org" );
    aboutData.addCredit( ki18n("Robert Moniot"), ki18n( "Fortran documentation" ), "moniot@fordham.edu" );
    aboutData.addCredit( ki18n("Ka-Ping Yee"), ki18n( "Python documentation utility" ), "ping@lfw.org" );
    aboutData.addCredit( ki18n("Dimitri van Heesch"), ki18n( "Doxygen wizard" ), "dimitri@stack.nl" );
    aboutData.addCredit( ki18n("Hugo Varotto"), ki18n( "Fileselector component" ), "hugo@varotto-usa.com" );
    aboutData.addCredit( ki18n("Matt Newell"), ki18n( "Fileselector component" ), "newellm@proaxis.com" );
    aboutData.addCredit( ki18n("Trolltech AS"), ki18n( "Designer code" ), "info@trolltech.com" );
    aboutData.addCredit( ki18n("Daniel Engelschalt"), ki18n( "C++ code completion, persistant class store" ), "daniel.engelschalt@gmx.net" );
    aboutData.addCredit( ki18n("Stephane Ancelot"), ki18n( "Patches" ), "sancelot@free.fr" );
    aboutData.addCredit( ki18n("Jens Zurheide"), ki18n( "Patches" ), "jens.zurheide@gmx.de" );
    aboutData.addCredit( ki18n("Luc Willems"), ki18n( "Help with Perl support" ), "Willems.luc@pandora.be" );
    aboutData.addCredit( ki18n("Marcel Turino"), ki18n( "Documentation index view" ), "M.Turino@gmx.de" );
    aboutData.addCredit( ki18n("Yann Hodique"), ki18n( "Patches" ), "Yann.Hodique@lifl.fr" );
    aboutData.addCredit( ki18n("Tobias Gl\303\244\303\237er") , ki18n( "Documentation Finder,  qmake projectmanager patches, usability improvements, bugfixes ... " ), "tobi.web@gmx.de" );
    aboutData.addCredit( ki18n("Andreas Koepfle") , ki18n( "QMake project manager patches" ), "koepfle@ti.uni-mannheim.de" );
    aboutData.addCredit( ki18n("Sascha Cunz") , ki18n( "Cleanup and bugfixes for qEditor, AutoMake and much other stuff" ), "mail@sacu.de" );
    aboutData.addCredit( ki18n("Robert Gruber") , ki18n( "SnippetPart, debugger and usability patches" ), "rgruber@users.sourceforge.net" );
    aboutData.addCredit( ki18n("Zoran Karavla"), ki18n( "Artwork for the ruby language" ), "webmaster@the-error.net", "http://the-error.net" );

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineOptions options;
    options.add("profile <profile>", ki18n( "Profile to load" ));
    options.add("project <project>", ki18n( "Project to load" ));
    options.add("+file(s)", ki18n( "Files to load" ));
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
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

    Core::initialize();
    KGlobal::locale()->insertCatalog( Core::self()->componentData().catalogName() );
    Core* core = Core::self();

    if ( splash )
    {
        QObject::connect(core->pluginController(), SIGNAL(loadingPlugin(QString)),
                         splash, SLOT(showMessage(QString)));
        QTimer::singleShot(0, splash, SLOT(deleteLater()));
        splash->showMessage( i18n( "Starting GUI" ) );
    }

    bool openedProject = false;
    QString projectName = args->getOption("project");
    if ( !projectName.isEmpty() )
    {
        core->projectController()->openProject( KUrl(projectName) );
        openedProject = true;
    }
    else if( args->count() > 0 )
    {
        for ( int i=0; i<args->count(); i++)
        {
            KUrl url = args->url( i );
            QString ext = QFileInfo( url.fileName() ).suffix();
            if( ext == "kdev4" )
            {
                core->projectController()->openProject( url );
                openedProject = true;
            }
        }
    }

    if( !openedProject )
    {
        for( int a=0; a<args->count(); ++a )
        {
            QString file = args->arg(a);
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

            if( KUrl::isRelativeUrl(file) )
            {
                KUrl u = QDir::currentPath();
                u.addPath(file);
                file = u.path();
            }
            core->documentController()->openDocument(KUrl(file), line);
        }
        if( splash && args->count() == 1 )
        {
            splash->showMessage(args->url(0).pathOrUrl());
        }
    }

    return app.exec();
}

