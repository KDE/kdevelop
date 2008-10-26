/***************************************************************************
 *   Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>                 *
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


#ifndef QTEST_KDE4ADDUNITTEST_H_INCLUDED
#define QTEST_KDE4ADDUNITTEST_H_INCLUDED

#include <QtCore/QObject>
#include <QtCore/QDir>
#include <KUrl>

class KDE4AddUnitTest : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void go();

private:
    QDir m_tmpProjectDir;
    KUrl m_projectFile;
};

#endif // QTEST_KDE4ADDUNITTEST_H_INCLUDED
