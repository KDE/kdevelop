/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SUBLIMEAREAWALKERS_H
#define KDEVPLATFORM_SUBLIMEAREAWALKERS_H

#include <QList>


namespace Sublime {

//area walkers implementations

template <typename Operator>
Area::WalkerMode Area::walkViewsInternal(Operator &op, AreaIndex *index)
{
    Area::WalkerMode mode = op(index);
    if (mode == Area::StopWalker)
        return mode;
    else if (index->first() && index->second())
    {
        mode = walkViewsInternal(op, index->first());
        if (mode == Area::StopWalker)
            return mode;
        mode = walkViewsInternal(op, index->second());
    }
    return mode;
}

template <typename Operator>
void Area::walkViews(Operator &op, AreaIndex *index)
{
    walkViewsInternal(op, index);
}

template <typename Operator>
void Area::walkToolViews(Operator &op, Positions positions)
{
    const QList<View*> currViews = toolViews();
    for (View* view : currViews) {
        Sublime::Position position = toolViewPosition(view);
        if (position & positions)
            if (op(view, position) == Area::StopWalker)
                break;
    }
}

}

#endif

