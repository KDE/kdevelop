/* KDevelop xUnit plugin
 *
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

#include <QCoreApplication>
#include <QTimer>
#include "qtestoutputparsertest.h"
#include <QTextStream>

namespace {
QTextStream cout(stdout);
}

void msgHandler(QtMsgType type, const char * msg)
{
    if (type == QtFatalMsg) {
        cout << endl << msg << endl;
    } else if (type == QtWarningMsg) {
        cout << endl << msg << endl;
    }
}

void usage(char** argv)
{
        cout << "QTestoutputparser stress test. Generates random XML data and feeds this piece by piece to a parser.\n"
             << "available commands:\n"
             << "valid    - valid qtest-xml, with result verification.\n"
             << "garbage  - qtest-xml interwoven with random garbage.\n\n"
             << "usage " << argv[0] << " [valid|garbage]" << endl;
        exit(-1);
}

int main(int argc, char** argv)
{
    if (argc == 1) usage(argv);
    QString test(argv[1]);
    if (test != "valid" && test != "garbage") usage(argv);
    bool quiet = (argc == 3 && !strcmp(argv[2], "--quiet"));

    QCoreApplication app(argc, argv);
    qRegisterMetaType<QModelIndex>("QModelIndex");
    qInstallMsgHandler(msgHandler);

    QTimer* timer = new QTimer;
    timer->setSingleShot(true);
    timer->setInterval(1);
    QTest::OutputParserTest* pst = new QTest::OutputParserTest;
    pst->setQuiet(quiet);
    if (test == "valid") {
        QObject::connect(timer, SIGNAL(timeout()), pst, SLOT(randomValidXML()));
    } else if (test == "garbage") {
        QObject::connect(timer, SIGNAL(timeout()), pst, SLOT(randomGarbageXML()));
    }
    timer->start();
    return app.exec();
}
