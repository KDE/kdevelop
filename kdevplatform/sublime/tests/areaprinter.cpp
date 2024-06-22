/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "areaprinter.h"

#include <sublime/view.h>
#include <sublime/areaindex.h>

using namespace Sublime;

// class AreaViewsPrinter

AreaViewsPrinter::AreaViewsPrinter()
{
    result = QStringLiteral("\n");
}

Area::WalkerMode AreaViewsPrinter::operator()(Sublime::AreaIndex *index)
{
    result += printIndentation(index) + "[ ";
    if (index->views().isEmpty())
        result += printOrientation(index->orientation()) + ' ';
    else
    {
        for (View* view : std::as_const(index->views())) {
            result += view->objectName() + ' ';
        }
    }
    result += QLatin1String("]\n");
    return Area::ContinueWalker;
}

QString AreaViewsPrinter::printIndentation(Sublime::AreaIndex *index) const
{
    QString i;
    while ((index = index->parent()))
        i += QLatin1String("    ");
    return i;
}

QString AreaViewsPrinter::printOrientation(Qt::Orientation o) const
{
    if (o == Qt::Vertical)
        return QStringLiteral("vertical splitter");
    else
        return QStringLiteral("horizontal splitter");
}



// class AreaToolViewsPrinter

AreaToolViewsPrinter::AreaToolViewsPrinter()
{
    result = QStringLiteral("\n");
}

Area::WalkerMode AreaToolViewsPrinter::operator()(Sublime::View *view, Sublime::Position position)
{
    result += view->objectName() + " [ " + printPosition(position) + " ]" + '\n';
    return Area::ContinueWalker;
}

QString AreaToolViewsPrinter::printPosition(Sublime::Position position)
{
    switch (position)
    {
        case Sublime::Left: return QStringLiteral("left");
        case Sublime::Right: return QStringLiteral("right");
        case Sublime::Bottom: return QStringLiteral("bottom");
        case Sublime::Top: return QStringLiteral("top");
        default: return QStringLiteral("wrong position");
    }
}

