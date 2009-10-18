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
#include <outputview/ioutputviewmodel.h>

#include <language/editor/editorrevisiontracker.h>

class QModelIndex;

using namespace KDevelop;

class GrepOutputItem : public QStandardItem
{
public:
    GrepOutputItem(const QString &fileName, const QString &lineNumber,
                   const QString &text, bool showFilename);
    ~GrepOutputItem();

    QString m_fileName, m_lineNumber, m_text;
    bool m_showFilename;
};

class GrepOutputModel : public QStandardItemModel, public KDevelop::IOutputViewModel
{
    Q_OBJECT

public:
    enum ItemType
    {
        Text,
        File
    };
    explicit GrepOutputModel( QObject *parent = 0 );
    ~GrepOutputModel();

    void setRegExp(const QString& regExp);

    void activate( const QModelIndex &idx );
    QModelIndex nextHighlightIndex( const QModelIndex& currentIndex );
    QModelIndex previousHighlightIndex( const QModelIndex& currentIndex );

public Q_SLOTS:
    void appendOutputs( const QStringList &lines );
    void appendErrors( const QStringList &lines );
    void slotCompleted();
    void slotFailed();

private:
    
    /** Check whether idx points to a row in the current model. */
    bool isValidIndex( const QModelIndex& idx ) const;
    
//     int m_matchCount;
    QString _lastfilename;
    QRegExp m_regExp;
    QString m_pattern;
    EditorRevisionTracker m_tracker;
};

#endif
