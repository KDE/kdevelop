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

#ifndef KDEVPLATFORM_FILTEREDITEM_H
#define KDEVPLATFORM_FILTEREDITEM_H

#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QUrl>

#include <outputview/outputviewexport.h>

namespace KDevelop
{

/**
 * Holds all metadata of a given compiler/script/whatever output line. E.g. if it is an error or
 * a warning, if it is clickable or not, line number, column number, etc.
 * Note that line and column number are stored as starting with with 0 (Your compiler/whatever tool 
 * may count from 1 ).
 **/
struct KDEVPLATFORMOUTPUTVIEW_EXPORT FilteredItem
{
    enum FilteredOutputItemType
    {
        InvalidItem = 0,
        ErrorItem = 1,
        WarningItem = 2,
        ActionItem = 3,
        CustomItem = 4,
        StandardItem = 5,
        InformationItem = 6
    };

    explicit FilteredItem( const QString& line = QString() );

    FilteredItem( const QString& line, FilteredOutputItemType type );

    QString originalLine;
    FilteredOutputItemType type;
    QString shortenedText;
    bool isActivatable;
    QUrl url;
    /// lineNo starts with 0
    int lineNo;
    /// columnNo starts with 0
    int columnNo;
};


} // namespace KDevelop

Q_DECLARE_METATYPE( KDevelop::FilteredItem::FilteredOutputItemType )
Q_DECLARE_TYPEINFO( KDevelop::FilteredItem, Q_MOVABLE_TYPE );

#endif // KDEVPLATFORM_FILTEREDITEM_H
