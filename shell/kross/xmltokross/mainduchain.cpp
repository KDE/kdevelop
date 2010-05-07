/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
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

#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>
#include <KDebug>

#include <QStringList>
#include <QFile>

#include "duchainextractor.h"

class ConsoleIDEExtension : public KDevelop::ShellExtension
{
    public:
        static void init() { s_instance = new ConsoleIDEExtension(); } 

        virtual QString xmlFile() { return QString(); }
        virtual QString binaryPath() { return QString(); };
        virtual QString defaultProfile() { return QString(); }
        virtual KDevelop::AreaParams defaultArea(){
            KDevelop::AreaParams params = {"code", i18n("Code")};
            return params;
        }
        virtual QString projectFileExtension() { return QString(); }
        virtual QString projectFileDescription() { return QString(); }
	virtual QStringList defaultPlugins() { return QStringList(); }

protected:
        ConsoleIDEExtension() {}
};

bool verbose=false;
void messageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
        case QtDebugMsg:
            if(verbose)
                fprintf(stderr, "%s\n", msg);
            break;
        case QtWarningMsg:
            if(verbose)
                fprintf(stderr, "Warning: %s\n", msg);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s\n", msg);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s\n", msg);
            abort();
    }
}

using namespace KDevelop;
int main(int argc, char** argv)
{
    qInstallMsgHandler(messageOutput);
    KAboutData aboutData( "duchaintokross", 0, ki18n( "duchaintokross" ),
                          "33", ki18n("Wrapper generator from C++ to Kross"), KAboutData::License_GPL,
                          ki18n( "(c) 2008, The KDevelop developers" ), KLocalizedString(), "http://www.kdevelop.org" );
    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineOptions options;
    options.add("+file", ki18n("input .cpp file"), QByteArray());
    options.add("I <includes>", ki18n("include directories for the parser (colon separated)"), QByteArray());
    options.add("i <includes>", ki18n("include directories that the generated implementation file will have (colon separated)"), QByteArray());
    options.add("b <builddir>", ki18n("build dir where the Makefile for the input is added"), QByteArray());
    options.add("F <filename>", ki18n("filename to be used for the .moc and the .h file"), QByteArray());
    options.add("D <directory>", ki18n("directory where to put the .h output"), QByteArray());
    options.add("o <output>", ki18n("directory where to put the code output"), QByteArray());
    options.add("impl", ki18n("create implementation"), QByteArray());
    options.add("verbose", ki18n("Verbose output"), QByteArray());
    KCmdLineArgs::addCmdLineOptions( options );
    
    KApplication app;
    ConsoleIDEExtension::init();
    Core::initialize(0, Core::NoUi);
    
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    KUrl::List includes;
    KUrl input;
    
    for(int i=0; i<args->count(); i++)
    {
        QString arg=args->arg(i);
        input=KUrl(arg);
    }
    
    foreach(const QString& inc, args->getOption("I").split(':'))
        includes << KUrl(inc);
    KUrl builddir(args->getOption("b"));
    QString filename(args->getOption("F"));
    QString directory(args->getOption("D"));
    QStringList toinclude(args->getOption("i").split(':'));
    QString output(args->getOption("o"));
    verbose=args->isSet("verbose");
    bool createImpl=args->isSet("impl");
    args->clear();
    
    DUChainExtractor e;
    kDebug() << input << builddir << includes << toinclude;
    e.setWriteImpl(createImpl);
    e.start(input, builddir, includes, filename, directory, toinclude, output);
    
    return app.exec();
}
