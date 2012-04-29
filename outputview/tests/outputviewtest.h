/*
    This file is part of KDevelop
    Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef OUTPUTVIEWTEST_H
#define OUTPUTVIEWTEST_H

#include <QObject>

class OutputViewTest : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

//     void testParseOrdering_simple();
//     void testParseOrdering_lockup();
//     void testParseOrdering_foregroundThread();
//     void testParseOrdering_noSequentialProcessing();
// 
//     void benchmark();
    
};

#endif // OUTPUTVIEWTEST_H
