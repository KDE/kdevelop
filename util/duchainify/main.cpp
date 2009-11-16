/***************************************************************************
 *   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>                         *
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

#include <shell/core.h>
#include <shell/shellextension.h>
#include <language/backgroundparser/parsejob.h>
#include <interfaces/ilanguage.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/ilanguagecontroller.h>

#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>
#include <KDE/KAboutData>
#include <KDE/KDebug>

#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <stdio.h>
#include <tests/autotestshell.h>
#include <QDirIterator>
#include <language/backgroundparser/backgroundparser.h>
#include "main.h"
#include <language/duchain/duchain.h>

bool verbose=false, warnings=false;

using namespace KDevelop;

void messageOutput(QtMsgType type, const char *msg)
{
    
    switch (type) {
        case QtDebugMsg:
            if(verbose)
                std::cerr << msg << std::endl;
            break;
        case QtWarningMsg:
            if(warnings)
                std::cerr << msg << std::endl;
            break;
        case QtCriticalMsg:
            std::cerr << msg << std::endl;
            break;
        case QtFatalMsg:
            std::cerr << msg << std::endl;
            abort();
    }
}


Manager::Manager()
{
    connect(ICore::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), SLOT(parseJobFinished(KDevelop::ParseJob*)));
}


void Manager::updateReady(IndexedString url, ReferencedTopDUContext topContext)
{
    kDebug() << "finished" << url.toUrl().toLocalFile() << "success: " << (bool)topContext;
    
    m_waiting.remove(url.toUrl());
    
    std::cout << "still waiting: " << m_waiting.size() << std::endl;
    
    if(m_waiting.isEmpty())
    {
        std::cout << "ready" << std::endl;
        QApplication::exit();
    }
}


void Manager::addToBackgroundParser(QString path, TopDUContext::Features features)
{
    QFileInfo info(path);
    
    if(info.isFile())
    {
        kDebug() << "adding file" << path;
        KUrl pathUrl(path);
        
        m_waiting << pathUrl;
        
        KDevelop::DUChain::self()->updateContextForUrl(KDevelop::IndexedString(pathUrl), features, this);
        
    }else if(info.isDir())
    {
        QDirIterator contents(path);
        while(contents.hasNext()) {
            QString newPath = contents.next();
            if(!newPath.endsWith("."))
                addToBackgroundParser(newPath, features);
        }
    }
}

QSet< KUrl > Manager::waiting()
{
    return m_waiting;
}

using namespace KDevelop;
int main(int argc, char** argv)
{
    qInstallMsgHandler(messageOutput);
    KAboutData aboutData( "duchainify", 0, ki18n( "duchainify" ),
                          "1", ki18n("Duchain builder application"), KAboutData::License_GPL,
                          ki18n( "(c) 2009 David Nolden" ), KLocalizedString(), "http://www.kdevelop.org" );
    KCmdLineArgs::init( argc, argv, &aboutData, KCmdLineArgs::CmdLineArgNone );
    KCmdLineOptions options;
    options.add("+dir", ki18n("directory"));
    
    options.add("warnings", ki18n("Show warnings"));
    options.add("verbose", ki18n("Show warnings and debug output"));
    options.add("force-update", ki18n("Enforce an update of the top-contexts corresponding to the given files"));
    options.add("threads <count>", ki18n("Number of threads to use"));
    options.add("f <features>", ki18n("Features to build. Options: visible-declarations (default), all-declarations, all-declarations-and-uses, all-declarations-and-uses-and-AST"));
    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    
    KApplication app;

    AutoTestShell::init();
    Core::initialize(KDevelop::Core::NoUi);
    
    
    KUrl::List includes;
    KUrl dir;
    
    if(args->count() == 0) {
        std::cerr << "Need directory to duchainify" << std::endl;
        exit(0);
    }
    
    for(int i=0; i<args->count(); i++)
    {
        QString arg=args->arg(i);
        dir=KUrl(arg);
    }

    verbose=args->isSet("verbose");
    warnings=args->isSet("warnings");

    uint features = TopDUContext::VisibleDeclarationsAndContexts;
    if(args->isSet("f"))
    {
        QString featuresStr = args->getOption("f");
        if(featuresStr == "visible-declarations")
        {
            features = TopDUContext::VisibleDeclarationsAndContexts;
        }
        else if(featuresStr == "all-declarations")
        {
            features = TopDUContext::AllDeclarationsAndContexts;
        }
        else if(featuresStr == "all-declarations-and-uses")
        {
            features = TopDUContext::AllDeclarationsContextsAndUses;
        }
        else if(featuresStr == "all-declarations-and-uses-and-AST")
        {
            features = TopDUContext::AllDeclarationsContextsUsesAndAST;
        }else{
            std::cerr << "Wrong feature-string given\n";
            exit(2);
        }
    }
    if(args->isSet("force-update"))
        features |= TopDUContext::ForceUpdate;
    
    if(args->isSet("threads"))
    {
        bool ok = false;
        int count = args->getOption("threads").toInt(&ok);
        ICore::self()->languageController()->backgroundParser()->setThreadCount(count);
        if(!ok) {
            std::cerr << "bad thread count\n";
            exit(0);
        }
    }
    
    Manager manager;
    
    args->clear();
    
    manager.addToBackgroundParser(dir.toLocalFile(), (TopDUContext::Features)features);
    
    std::cout << "Added " << manager.waiting().size() << " files to the background parser" << std::endl;
    
    app.exec();
}

#include "main.moc"
