/***************************************************************************
 *   Copyright (C) 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
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

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

#define KDEVTEST_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
    static const char description[] = "Sublime Library Test"; \
    KAboutData aboutData("test", 0, ki18n("Test"), \
                         "1.0", ki18n(description), KAboutData::License_LGPL, ki18n(\
                         "(c) 2007, KDevelop Developers"), KLocalizedString(), "http://www.kdevelop.org" ); \
 \
    KCmdLineArgs::init(argc, argv, &aboutData); \
    KApplication app; \
 \
    TestObject tc; \
    return QTest::qExec(&tc, argc, argv); \
}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
