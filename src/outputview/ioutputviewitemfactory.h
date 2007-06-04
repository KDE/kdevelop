/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn  <dukjuahn@gmail.com>                 *
 *   Copyright (C) 2007 by Andreas Pakulat  <apaku@gmx.de>                 *
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

#ifndef IOUTPUTVIEWITEMFACTORY_H
#define IOUTPUTVIEWITEMFACTORY_H

#include "outputviewexport.h"
#include <QtCore/QString>
#include <ioutputviewitem.h>

class KDEVPLATFORMOUTPUTVIEW_EXPORT IOutputViewItemFactory
{
public:
    virtual ~IOutputViewItemFactory();
    /**
     * Creates new item depending on @arg lineOutput.
     * The item may use different appearance depending on the types of output.
     * @return IOutputViewItem object which will be appended at the end of the OutputView.
     * @sa IOutputViewItem
     */
    virtual IOutputViewItem* createItem( const QString& lineOutput ) = 0;
};

template<typename T>
class KDEVPLATFORMOUTPUTVIEW_EXPORT StandardOutputViewItemFactory : public IOutputViewItemFactory
{
public:
    StandardOutputViewItemFactory()
    {}
    /**
     * Creates new item depending on @arg lineOutput.
     * The item may use different appearance depending on the types of output.
     * @return IOutputViewItem object which will be appended at the end of the OutputView.
     * @sa IOutputViewItem
     */
    virtual IOutputViewItem* createItem( const QString& lineOutput )
    {
        return new T( lineOutput );
    }
};


#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
