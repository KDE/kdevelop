/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IOUTPUTVIEWITEMFACTORY_H
#define IOUTPUTVIEWITEMFACTORY_H

#include "kdevexport.h"

class IOutputViewItem;
class QString;
class QStandardItem;

class OUTPUTVIEWINTERFACES_EXPORT IOutputViewItemFactory
{
public:
    /// Destructor
    virtual ~IOutputViewItemFactory();

    /**
     * Parses @arg lineOutput from KProcess and generates appropriate item.
     * May use different appearance depending on the types of output.
     * @return IOutputViewItem object which will be appended at the end of the OutputView.
     * @sa IOutputViewItem
     */
    virtual QStandardItem* createItem( const QString& lineOutput ) = 0;
};

#endif
