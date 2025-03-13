/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
private Q_SLOTS:
    void initTestCase();
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
        const QString &areas, int containers, int splitters, const QString& location=QString());

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
