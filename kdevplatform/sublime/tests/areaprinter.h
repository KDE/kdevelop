/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_AREAPRINTER_H
#define KDEVPLATFORM_AREAPRINTER_H


#include <sublime/area.h>
#include <sublime/sublimedefs.h>

namespace Sublime {
    class AreaIndex;
    class View;
}

//those two classes will pretty-print area views and tool views
//make sure you provided object names for your views (with setObjectName())

class AreaViewsPrinter {
public:
    AreaViewsPrinter();
    Sublime::Area::WalkerMode operator()(Sublime::AreaIndex *index);
    QString result;

private:
    QString printIndentation(Sublime::AreaIndex *index) const;
    QString printOrientation(Qt::Orientation o) const;
};

class AreaToolViewsPrinter {
public:
    AreaToolViewsPrinter();
    Sublime::Area::WalkerMode operator()(Sublime::View *view, Sublime::Position position);
    QString result;

private:
    QString printPosition(Sublime::Position position);
};

#endif

