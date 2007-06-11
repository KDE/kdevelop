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

#ifndef MAKEITEM_H
#define MAKEITEM_H

#include <QStandardItem>
class QAction;
template<typename T1> class QList;
class MakeBuilder;

// class MakeItem : public IOutputViewItem
// {
// public:
//     MakeItem( const QString &text );
//     virtual ~MakeItem();
//
//     virtual void activated();
//     virtual QList<QAction*> contextMenuActions();
// };

class MakeWarningItem : public QStandardItem
{
public:
    MakeWarningItem( const QString &text );
    virtual ~MakeWarningItem();

//     virtual void activate();

    // used to open a docuement which caused warning or error
    QString file;
    int lineNo;
    QString errorText;

protected:
    const MakeBuilder *m_builder;
};

class MakeErrorItem : public MakeWarningItem
{
public:
    MakeErrorItem( const QString &text );
    virtual ~MakeErrorItem();

};

class MakeActionItem : public QStandardItem
{
public:
    MakeActionItem( const QString& action, const QString& file, const QString& tool, const QString& line );
    virtual ~MakeActionItem();

// private:
//     QString m_action;
//     QString m_file;
//     QString m_tool;
};



#endif
