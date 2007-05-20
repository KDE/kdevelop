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

#ifndef MAKEITEMFACTORY_H
#define MAKEITEMFACTORY_H

#include "ioutputviewitemfactory.h"
class IOutputViewItem;
class MakeBuilder;

class MakeItemFactory : public IOutputViewItemFactory
{
public:
    MakeItemFactory( const MakeBuilder *builder );
    virtual ~MakeItemFactory();

    QStandardItem* createItem( const QString& lineOutput );

private:
    const MakeBuilder *m_builder;
};

#endif
