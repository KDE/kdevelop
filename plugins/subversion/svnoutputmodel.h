/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_OUTPUTMODEL_H
#define SVN_OUTPUTMODEL_H

#include <QAbstractItemModel>
#include <kurl.h>
#include <outputview/ioutputviewmodel.h>
#include <QStandardItemModel>

/**
 * Model that is used by outputview notification.
 * When activated, open the file. Depending on the result of update operation, it paints
 * with corresponding color. For example, regarding conflicted item, it paints red color
 * on that file.
 *
 * TODO Clearing the list by context menu.
 */

class KDevSvnPlugin;

class SvnOutputItem : public QStandardItem
{
public:
    explicit SvnOutputItem( const QString &path, const QString &msg );
    virtual ~SvnOutputItem();
    bool stopHere();
    QString m_path;
    QString m_msg;
    bool m_stop;
};

class SvnOutputModel : public QStandardItemModel, public KDevelop::IOutputViewModel
{
    Q_OBJECT
public:
    enum UpdateType
    {
        Conflict
    };

    explicit SvnOutputModel( KDevSvnPlugin *part, QObject *parent = 0 );
    ~SvnOutputModel();

    // IOutputViewModel interfaces.
    /// Open the file
    void activate( const QModelIndex& index );

    /// Highlight next conflicted item in list.
    QModelIndex nextHighlightIndex( const QModelIndex& currentIndex );

    /// Highlight previous conflicted item in list.
    QModelIndex previousHighlightIndex( const QModelIndex& currentIndex );

private:
    KDevSvnPlugin *m_part;
};

#endif
