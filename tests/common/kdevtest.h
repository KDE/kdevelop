/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
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
#ifndef KDEVTEST
#define KDEVTEST

#include <stdlib.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

#define KDEVTEST_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
    setenv("LC_ALL", "C", 1); \
    setenv("KDEHOME", QFile::encodeName( QDir::homePath() + "/.kde-unit-test" ), 1); \
    setenv("XDG_DATA_HOME", QFile::encodeName( QDir::homePath() + "/.kde-unit-test/xdg/local" ), 1); \
    setenv("XDG_CONFIG_HOME", QFile::encodeName( QDir::homePath() + "/.kde-unit-test/xdg/config" ), 1); \
 \
    static const char description[] = "KDevelop Test"; \
    KAboutData aboutData("kdevtest", 0, ki18n("KDevelop Test"), \
                         "1.0", ki18n(description), KAboutData::License_LGPL, ki18n(\
                         "(c) 2007-2008, KDevelop Developers"), KLocalizedString(), "http://www.kdevelop.org" ); \
 \
    KCmdLineArgs::init(argc, argv, &aboutData); \
    KCmdLineOptions options; \
    options.add("+[functionNames]", ki18n("Run only specified test functions")); \
    options.add("functions", ki18n("Show all test functions available in the test")); \
    options.add("silent", ki18n("Silent output, only show warnings, failures and minimal status messages")); \
    options.add("v1", ki18n("Verbose output, show information on entering and exiting test functions")); \
    options.add("v2", ki18n("Extended verbose output, also show each QCOMPARE() and QVERIFY()")); \
    options.add("vs", ki18n("Show every signal that gets emitted")); \
    options.add("xml", ki18n("Output XML formatted results instead of plain text")); \
    options.add("lightxml", ki18n("Output results as a stream of XML tags")); \
    options.add("o <file>", ki18n("Write output to the specified file, rather than to standard output")); \
    options.add("eventdelay <ms>", ki18n("Default delay for keyboard or mouse simulation")); \
    options.add("keydelay <ms>", ki18n("Like --eventdelay, but only influences keyboard simulation and not mouse simulation")); \
    options.add("mousedelay <ms>", ki18n("Like --eventdelay, but only influences mouse simulation and not keyboard simulation.")); \
    options.add("keyevent-verbose", ki18n("Show more verbose output for keyboard simulation")); \
    options.add("maxwarnings <number>", ki18n("Set the maximum number of warnings to output. 0 for unlimited, defaults to 2000")); \
    KCmdLineArgs::addCmdLineOptions( options ); \
    KApplication app; \
 \
    TestObject tc; \
    return QTest::qExec(&tc, argc, argv); \
}

#endif

