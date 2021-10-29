/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_FILTEREDITEM_H
#define KDEVPLATFORM_FILTEREDITEM_H

#include <QMetaType>
#include <QString>
#include <QUrl>

#include <outputview/outputviewexport.h>

namespace KDevelop
{

/**
 * Holds all metadata of a given compiler/script/whatever output line. E.g. if it is an error or
 * a warning, if it is clickable or not, line number, column number, etc.
 * Note that line and column number are stored as starting with 0 (Your compiler/whatever tool
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
    FilteredOutputItemType type =  InvalidItem;
    bool isActivatable = false;
    QUrl url;
    /// lineNo starts with 0
    int lineNo = -1;
    /// columnNo starts with 0
    int columnNo = -1;
};


} // namespace KDevelop

Q_DECLARE_METATYPE( KDevelop::FilteredItem )
Q_DECLARE_METATYPE( KDevelop::FilteredItem::FilteredOutputItemType )
Q_DECLARE_TYPEINFO( KDevelop::FilteredItem, Q_MOVABLE_TYPE );

#endif // KDEVPLATFORM_FILTEREDITEM_H
