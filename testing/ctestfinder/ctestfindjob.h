/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Miha ?an?ula <miha.cancula@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef CTESTFINDJOB_H
#define CTESTFINDJOB_H

#include <KJob>

class CTestSuite;

class CTestFindJob : public KJob
{
    Q_OBJECT
    
public:
    explicit CTestFindJob(CTestSuite* suite, QObject* parent = 0);
    virtual void start();
    
private slots:
    virtual void findTestCases();
    
private:
    CTestSuite* m_suite;
};

#endif // CTESTFINDJOB_H
