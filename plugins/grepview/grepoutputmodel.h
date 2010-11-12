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
#include <language/codegen/documentchangeset.h>
#include <kde_file.h>

class QModelIndex;

namespace KDevelop {
    class IStatus;
}

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

    GrepOutputItem(KDevelop::DocumentChangePointer change, const QString &text, bool replace);
    GrepOutputItem(const QString &filename, const QString &text);
    ~GrepOutputItem();

    QString filename() const ;
    int lineNumber() const ;
    KDevelop::DocumentChangePointer change() const ;
    bool collapsed() const ;
    bool expanded() const ;
    bool isText() const { return data()==Text; }
    bool isMatch() const;
    bool collapse();
    bool expand();
    bool toggleView();

private:
    KDevelop::DocumentChangePointer m_change;
   
    void showCollapsed();
    void showExpanded();
};

Q_DECLARE_METATYPE(GrepOutputItem::List);

class GrepOutputModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit GrepOutputModel( QObject *parent = 0 );
    ~GrepOutputModel();

    void setRegExp(const QRegExp& re);
  
	// the next three methods are currently not used, I need to investigate to know if we still need them
    QModelIndex nextHighlightIndex( const QModelIndex& currentIndex );
    QModelIndex previousHighlightIndex( const QModelIndex& currentIndex );

public Q_SLOTS:
    void appendOutputs( const QString &filename, const GrepOutputItem::List &lines );
    void activate( const QModelIndex &idx );
    void showErrorMessage( const QString& errorMessage );
    void showMessage( KDevelop::IStatus*, const QString& message );

private:
    
    /** Check whether idx points to a row in the current model. */
    bool isValidIndex( const QModelIndex& idx ) const;

    QRegExp m_regExp;

private slots:
    void updateCheckState(QStandardItem*);
};

#endif
