/***************************************************************************
 *   Copyright 2021 Christoph Roick <chrisito@gmx.de>                      *
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

#ifndef KDEVPLATFORM_TEST_WORKINGSETS_H
#define KDEVPLATFORM_TEST_WORKINGSETS_H

#include <QObject>
#include <QTemporaryDir>
#include <QTemporaryFile>

class QWidget;

namespace KDevelop
{
class WorkingSetController;
class IDocumentController;
}

namespace Sublime
{
class Area;
}

class TestWorkingSetController : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanupTestCase();
    void createWorkingSet();
    void deleteWorkingSet();
    void switchArea();

private:
    void restartSession();

    QWidget* m_closedSets;
    KDevelop::WorkingSetController* m_workingSetCtrl;
    Sublime::Area* m_area;
    Sublime::Area* m_area_debug;
    KDevelop::IDocumentController* m_documentCtrl;

    QTemporaryDir m_tempDir;
    QTemporaryFile m_file;
};

#endif // KDEVPLATFORM_TEST_WORKINGSETS_H
