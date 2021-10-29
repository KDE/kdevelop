/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_VIEWACTIVATION_H
#define KDEVPLATFORM_TEST_VIEWACTIVATION_H

#include <QObject>

namespace Sublime {
class View;
class Controller;
class Document;
class Area;
}

class TestViewActivation: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void viewActivation();
    void activationInMultipleMainWindows();
    void activationAfterViewRemoval();
    void activationAfterRemovalSimplestCase();
    void signalsOnViewCreationAndDeletion();

private:
    Sublime::Controller *controller;

    Sublime::Area *area;

    Sublime::Document *doc1;
    Sublime::Document *doc2;
    Sublime::Document *doc3;
    Sublime::Document *doc4;
    Sublime::Document *tool1;
    Sublime::Document *tool2;
    Sublime::Document *tool3;

    Sublime::View *view211;
    Sublime::View *view212;
    Sublime::View *view221;
    Sublime::View *view231;
    Sublime::View *view241;
    Sublime::View *viewT11;
    Sublime::View *viewT21;
    Sublime::View *viewT31;
    Sublime::View *viewT32;
};

#endif
