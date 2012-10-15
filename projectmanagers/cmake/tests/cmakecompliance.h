/* KDevelop CMake Support
 *
 * Copyright 2007 Aleix Pol Gonzalez <aleixpol@gmail.com>
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

#ifndef CMAKEMESSAGEASTTEST_H
#define CMAKEMESSAGEASTTEST_H

#include <QtTest/QtTest>

class CMakeProjectVisitor;
class CMakeCompliance : public QObject
{
    Q_OBJECT
    public:
        CMakeCompliance();
        virtual ~CMakeCompliance();
        static void addOutput(const QString& msg);

    private slots:
        void testEnumerate();
        void testEnumerate_data();
        
        void testCMakeTests();
        void testCMakeTests_data();
        
    private:
        CMakeProjectVisitor parseFile( const QString& sourcefile );
        static QString output;
};

#endif
