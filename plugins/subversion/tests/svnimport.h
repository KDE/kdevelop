/***************************************************************************
*   This file is part of KDevelop                                         *
*   Copyright 2009 Fabian Wiesel <fabian.wiesel@googlemail.com>           *
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
#ifndef KDEVPLATFORM_PLUGIN_SVNIMPORT_H
#define KDEVPLATFORM_PLUGIN_SVNIMPORT_H
#include <QtCore/QObject>

class KTempDir;
namespace KDevelop
{
class TestCore;
class ICentralizedVersionControl;
}

class SvnImport
            : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void testBasic();
    void testImportIntoDir();
    void testImportWithMissingDirs();
private:
    void validateImport( const QString& repo, KTempDir& checkout, const QString& origcontent );
    KDevelop::TestCore* core;
    KDevelop::ICentralizedVersionControl* vcs;
};

#endif // KDEVPLATFORM_PLUGIN_SVNRECURSIVEADD_H
