/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann and the KDevelop Team       *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "makeitemfactory.h"
#include "makeitem.h"
#include "outputfilters.h"
#include <QRegExp>
#include <QApplication>
#include <QPalette>

MakeItemFactory::MakeItemFactory( const MakeBuilder *builder )
    : IOutputViewItemFactory()
    , m_builder(builder)
{
    ErrorFilter *errFilter = new ErrorFilter(builder);
    m_filters.append( errFilter );

    MakeActionFilter *actFilter = new MakeActionFilter();
    m_filters.append( actFilter );
}

MakeItemFactory::~MakeItemFactory()
{
    Q_FOREACH( OutputFilter *filter, m_filters )
    {
        delete filter;
    }
}

IOutputViewItem* MakeItemFactory::createItem( const QString& line )
{
    Q_FOREACH( OutputFilter *filter, m_filters )
    {
        IOutputViewItem *item = filter->processAndCreate( line );
        if( item )
            return item;
    }

    // didn't match with any kind of filter. return default.
    IOutputViewItem *ret =  new IOutputViewItem( line );
    return ret;
}

