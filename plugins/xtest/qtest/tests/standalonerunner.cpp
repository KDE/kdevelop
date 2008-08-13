/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <QFile>
#include <QMainWindow>
#include <KApplication>
#include <KComponentData>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>

#include <runnerwindow.h>
#include "../xmlregister.h"
#include <runnermodel.h>

using namespace Veritas;
using namespace QTest;

void do_stuff(int argc, char** argv);
void do_useful_stuff(char** argv);

int main(int argc, char** argv)
{
    do_stuff(argc, argv);
    KApplication app;
    do_useful_stuff(argv);
    return app.exec();
}


#include <KDebug>

class Stuff : public QObject
{
Q_OBJECT
public slots:
    void showResultsWindow() {
        QMainWindow *window = new QMainWindow(0);
        main->resultsWidget()->setParent(window);
        window->setCentralWidget(main->resultsWidget());
        window->setParent(main);
        window->show();
    }
private:
    RunnerWindow* main;
};

#include "stuff.moc"

void do_useful_stuff(char** argv)
{
    QFile f(argv[1]);
    QTestRegister r;
    r.setRootDir(argv[2]);
    r.addFromXml(&f);
    RunnerModel* m = new RunnerModel;
    m->setRootItem(r.rootItem());
    RunnerWindow* w = new RunnerWindow;
    w->setModel(m);
    w->show();
    Stuff* s = new Stuff;
    QObject::connect(w, SIGNAL(openResultsView(bool)),
            s, SLOT(showResultsWindow()));
}

QByteArray b("stuff");
KAboutData* ad;
KCmdLineOptions* o;

void do_stuff(int argc, char** argv)
{
/*    KAboutData (
    const QByteArray &appName, 
    const QByteArray &catalogName, 
    const KLocalizedString &programName, 
    const QByteArray &version, 
    const KLocalizedString &shortDescription=KLocalizedString(), 
    enum LicenseKey licenseType=License_Unknown, 
    const KLocalizedString &copyrightStatement=KLocalizedString(), 
    const KLocalizedString &text=KLocalizedString(), 
    const QByteArray &homePageAddress=QByteArray(), 
    const QByteArray &bugsEmailAddress="submit@bugs.kde.org")*/

    ad = new KAboutData(b, b, ki18n(b), b, ki18n(b),
                        KAboutData::License_Unknown,
                        ki18n(b), ki18n(b), b, b);
    KCmdLineArgs::init(argc, argv, ad);
    o = new KCmdLineOptions;
    o->add("+regxml", ki18n("Test registration XML."));
    o->add("+testroot", ki18n("Test root."));
    KCmdLineArgs::addCmdLineOptions(*o);
}
