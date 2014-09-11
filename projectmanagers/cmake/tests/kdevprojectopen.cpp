/* This file is part of KDevelop
    Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QApplication>
#include <KLocalizedString>
#include <KAboutData>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <interfaces/iprojectcontroller.h>
#include "kdevprojectopen.h"
#include "testhelpers.h"
#include <QDebug>
#include <QCommandLineParser>

using namespace KDevelop;

KDevProjectOpen::KDevProjectOpen(QObject* parent)
    : QObject(parent)
    , m_toOpen(0)
{
    AutoTestShell::init();
    TestCore::initialize();

    cleanup();
    connect(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), SLOT(projectDone(KDevelop::IProject*)));
}

void KDevProjectOpen::cleanup()
{
    foreach(IProject* p, ICore::self()->projectController()->projects()) {
        ICore::self()->projectController()->closeProject(p);
    }
    Q_ASSERT(ICore::self()->projectController()->projects().isEmpty());
}

void KDevProjectOpen::openProject(const QUrl& path)
{
    const TestProjectPaths paths = projectPaths(path.toLocalFile());
    defaultConfigure(paths);

    m_toOpen++;
    ICore::self()->projectController()->openProject(paths.projectFile.toUrl());
}

void KDevProjectOpen::projectDone(IProject* )
{
    m_toOpen--;
    if(m_toOpen==0) {
        cleanup();
        qApp->exit();
    }
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    KAboutData aboutData( "kdevprojectopen", i18n( "KDevelop" ), "0.99", i18n("opens a project then closes it, debugging tool"), KAboutLicense::GPL,
                          i18n( "Copyright 1999-2012, The KDevelop developers" ), QString(), "http://www.kdevelop.org/" );
    aboutData.addAuthor( i18n("Aleix Pol Gonzalez"), i18n( "" ), "aleixpol@kde.org" );

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addPositionalArgument("projects", i18n("Projects to load"));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    if(parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    KDevProjectOpen opener;
    foreach(const QString& arg, parser.positionalArguments()) {
        opener.openProject(QUrl::fromUserInput(arg));
    }
    return app.exec();
}
