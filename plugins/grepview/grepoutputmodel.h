/***************************************************************************
 *   Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team              *
 *   bernd@kdevelop.org                                                    *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GREPOUTPUTMODEL_H
#define GREPOUTPUTMODEL_H

#include <QStandardItemModel>
#include <QRegExp>
#include <QList>
#include <outputview/ioutputviewmodel.h>

class QModelIndex;


class GrepOutputItem : public QStandardItem
{
public:
    typedef QList<GrepOutputItem> List;

    enum ItemType
    {
        Text,
        FileCollapsed,
        FileExpanded
    };

    GrepOutputItem(const QString &fileName, int lineNumber,
                   const QString &text);
    ~GrepOutputItem();

    QString filename() const ;
    int lineNumber() const ;
    bool collapsed() const ;
    bool expanded() const ;
    bool isText() const { return data()==Text; }
    bool collapse();
    bool expand();
    bool toggleView();

private:
    QString m_fileName, m_text;
    int m_lineNumber;
    void showCollapsed();
    void showExpanded();
};

class GrepOutputModel : public QStandardItemModel, public KDevelop::IOutputViewModel
{
    Q_OBJECT

public:
    explicit GrepOutputModel( QObject *parent = 0 );
    ~GrepOutputModel();

    void setRegExp(const QRegExp& re);

    void activate( const QModelIndex &idx );
    QModelIndex nextHighlightIndex( const QModelIndex& currentIndex );
    QModelIndex previousHighlightIndex( const QModelIndex& currentIndex );

public Q_SLOTS:
    void appendOutputs( const QString &filename, const GrepOutputItem::List &lines );

private:
    
    /** Check whether idx points to a row in the current model. */
    bool isValidIndex( const QModelIndex& idx ) const;

    QRegExp m_regExp;
};

#endif
