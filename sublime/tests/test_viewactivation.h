/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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
private slots:
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
