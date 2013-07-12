/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                     *
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

#ifndef KDEVPLATFORM_IOUTPUTVIEWMODEL_H
#define KDEVPLATFORM_IOUTPUTVIEWMODEL_H

#include "outputviewexport.h"
class QModelIndex;

namespace KDevelop
{

class KDEVPLATFORMOUTPUTVIEW_EXPORT IOutputViewModel
{
public:
    virtual ~IOutputViewModel();

    /**
     * Called when the index @arg index was activated in output view.
     */
    virtual void activate( const QModelIndex& index ) = 0;

    /**
     * Called when the user wants to see next item. For example, in makebuilder it would be
     * next error spot. In subversion plugin it would be the next conflicted item.
     *
     * @param currentIndex Currently selected index in active outputview. It can be invalid index
     *  if no item is selected or highlighted.
     * @return Next model index that is to be highlighted and activated.
     *  Return invalid index if no appropriate item to highlight exists.
     */
    virtual QModelIndex nextHighlightIndex( const QModelIndex& currentIndex ) = 0;

    /**
     * Called when the user wants to see previous item. For example, in makebuilder it would be
     * previous error spot. In subversion plugin it would be the previous conflicted item.
     *
     * @param currentIndex Currently selected index in active outputview. It can be invalid index
     *  if no item is selected or highlighted.
     * @return Previous model index that is to be highlighted and activated.
     *  Return invalid index if no appropriate item to highlight exists.
     */
    virtual QModelIndex previousHighlightIndex( const QModelIndex& currentIndex ) = 0;

};

}

#endif

