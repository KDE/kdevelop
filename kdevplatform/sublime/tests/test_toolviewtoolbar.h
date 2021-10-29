/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_TOOLVIEWTOOLBAR_H
#define KDEVPLATFORM_TEST_TOOLVIEWTOOLBAR_H

#include <QObject>

namespace Sublime {
class View;
class Controller;
class ToolDocument;
class Area;
}
class QToolBar;

class TestToolViewToolBar : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void horizontalTool();
    void verticalTool();
    void toolViewMove();

private:
    QToolBar* fetchToolBarFor(Sublime::View*);
    void assertGoodBar(QToolBar*, const QString& actionText);

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
