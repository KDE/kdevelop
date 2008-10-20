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

#include "standalonerunner.h"
#include "../xmlregister.h"

#include <veritas/runnertesthelper.h>

#include <QFile>
#include <QMainWindow>
#include <KApplication>
#include <KComponentData>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KDebug>
#include <QTimer>
#include <QTime>
#include <iostream>

using namespace Veritas;
using namespace QTest;

void do_stuff(int argc, char** argv);
bool doProfile;
QTime timer;

int main(int argc, char** argv)
{
    do_stuff(argc, argv);
    KApplication app;
    Boot* b = new Boot;
    b->regXML = QString(argv[1]);
    b->rootDir = QString(argv[2]);
    doProfile = (argc == 4);
    b->start();
    return app.exec();
}

void Boot::start()
{
    QTimer* t = new QTimer;
    t->setSingleShot(true);
    t->setInterval(100);
    connect(t, SIGNAL(timeout()), this, SLOT(load()));
    t->start();
}

void Boot::load()
{
    QFile f(regXML);
    m_reg = new XmlRegister;
    m_reg->setRootDir(rootDir);
    m_reg->setSource(&f);
    m_reg->reload();
    connect(m_reg, SIGNAL(reloadFinished(Veritas::Test*)),
            this, SLOT(showWindow()));

    QTimer* t = new QTimer;
    t->setSingleShot(true);
    t->setInterval(250);
    connect(t, SIGNAL(timeout()), this, SLOT(showWindow()));
    t->start();
}

void Boot::showWindow()
{
    kDebug() << "";
    RunnerTestHelper* runner = new RunnerTestHelper;
    runner->initializeGUI();
    runner->setRoot(m_reg->root());
    QMainWindow* mw = new QMainWindow;
    mw->setCentralWidget(runner->runnerWidget());
    mw->show();
    if (doProfile) {
        timer.start();
        runner->setTimeout(-1);
        runner->runTests();
        std::cerr <<  timer.elapsed() << std::endl;
        KApplication::exit();
    }
}

QByteArray b("stuff");
KAboutData* ad;
KCmdLineOptions* o;

void do_stuff(int argc, char** argv)
{
    ad = new KAboutData(b, b, ki18n(b), b, ki18n(b),
                        KAboutData::License_Unknown,
                        ki18n(b), ki18n(b), b, b);
    KCmdLineArgs::init(argc, argv, ad);
    o = new KCmdLineOptions;
    o->add("+regxml", ki18n("Test registration XML."));
    o->add("+testroot", ki18n("Test root."));
    o->add("+profile", ki18n("profile"));
    KCmdLineArgs::addCmdLineOptions(*o);
}

#include "standalonerunner.moc"
