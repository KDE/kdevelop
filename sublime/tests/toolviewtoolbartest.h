/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
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

#ifndef KDEVPLATFORM_TOOLVIEWTOOLBARTEST_H
#define KDEVPLATFORM_TOOLVIEWTOOLBARTEST_H

#include <QObject>

namespace Sublime {
class View;
class Controller;
class ToolDocument;
class Area;
}
class QDockWidget;
class QToolBar;

class ToolViewToolBarTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void horizontalTool();
    void verticalTool();
    void toolViewMove();

private:
    QToolBar* fetchToolBarFor(Sublime::View*);
    void assertGoodBar(QToolBar*, QString actionText);

private:
    Sublime::Controller *controller;
    Sublime::Area *area;
    Sublime::ToolDocument *tool1;
    Sublime::ToolDocument *tool2;
    Sublime::View *viewT11;
    Sublime::View *viewT21;
    QString actionTextT1;
    QString actionTextT2;
};

#endif
