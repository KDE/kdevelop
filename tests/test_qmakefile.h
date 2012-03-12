/* KDevelop QMake Support
 *
 * Copyright 2010 Milian Wolff <mail@milianw.de>
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

#ifndef TEST_QMAKEFILE_H
#define TEST_QMAKEFILE_H

#include <QObject>

class TestQMakeFile : public QObject {
    Q_OBJECT

private slots:
    void varResolution_data();
    void varResolution();

    void referenceParser_data();
    void referenceParser();

    void libTarget_data();
    void libTarget();

    void defines_data();
    void defines();

    void replaceFunctions_data();
    void replaceFunctions();

    void qtIncludeDirs_data();
    void qtIncludeDirs();

    void testInclude();

    void globbing_data();
    void globbing();

    void benchGlobbing();
};

#endif // TEST_QMAKEFILE_H
