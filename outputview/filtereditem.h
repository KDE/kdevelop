/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com        *
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

#ifndef FILTEREDITEM_H
#define FILTEREDITEM_H

#include <QString>
#include <KUrl>

namespace KDevelop
{

/** Holds all metadata of a given compiler output line. E.g. if it is an error or 
 * a warning, if it is clickable or not, etc. 
 **/
class FilteredItem
{
    public:
        enum FilteredOutputItemType
        {
            ErrorItem = 1,
            WarningItem = 2,
            ActionItem = 3,
            CustomItem = 4,
            StandardItem = 5,
            InformationItem = 6
        };

        FilteredItem( const QString& line );

        QString originalLine;
        QVariant type;
        QString shortenedText;
        bool isActivatable;
        KUrl url;
        int lineNo;
        int columnNo;
};


} // namespace KDevelop

Q_DECLARE_METATYPE( KDevelop::FilteredItem::FilteredOutputItemType )

#endif // FILTEREDITEM_H
