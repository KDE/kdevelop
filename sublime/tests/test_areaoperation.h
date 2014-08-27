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
#ifndef KDEVPLATFORM_TEST_AREAOPERATION_H
#define KDEVPLATFORM_TEST_AREAOPERATION_H

#include <QObject>

namespace Sublime {
    class Area;
    class View;
    class Controller;
    class MainWindow;
}

class TestAreaOperation: public QObject {
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void areaConstruction();
    void mainWindowConstruction();
    void areaCloning();
    void areaSwitchingInSameMainwindow();
    void simpleViewAdditionAndDeletion();
    void complexViewAdditionAndDeletion();
    void toolViewAdditionAndDeletion();
    void testAddingViewAfter();
    void splitViewActiveTabsTest();

private:
    void checkArea1(Sublime::MainWindow *mw);
    void checkArea2(Sublime::MainWindow *mw);
    /*! @param location short descriptive message printed on failure. */
    void checkAreaViewsDisplay(Sublime::MainWindow *mw, Sublime::Area *area,
        const QString &areas, int containers, int splitters, QString location=QString());

    Sublime::View *findNamedView(Sublime::Area *area, const QString &name);

    Sublime::Controller *m_controller;

    Sublime::Area *m_area1;
    Sublime::Area *m_area2;
    Sublime::Area *m_area3;

    Sublime::View *m_pView111;
    Sublime::View *m_pView121;
    Sublime::View *m_pView122;
    Sublime::View *m_pView131;
};

#endif
