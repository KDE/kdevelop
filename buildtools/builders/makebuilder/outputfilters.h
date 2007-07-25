/***************************************************************************
 *   Copyright 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>               *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef OUTPUTFILTERS_H
#define OUTPUTFILTERS_H

class IOutputViewItem;
class MakeBuilder;
class QString;
// template <typename T1> class QList;
#include <QList>
class ErrorFormat;
class ActionFormat;
class QStandardItem;

class ErrorFilter
{
public:
    ErrorFilter( );
    virtual ~ErrorFilter();

    virtual QStandardItem* processAndCreate( const QString& line );

private:
    QList<ErrorFormat> m_errList;
};

class MakeActionFilter
{
public:
    MakeActionFilter();
    virtual ~MakeActionFilter();

    QStandardItem* processAndCreate( const QString& line );
private:
    QList<ActionFormat> m_actlist;
};

/// TODO read regexps from project file (.kdev4)
class CustomFilter
{
public:
    CustomFilter();
    virtual ~CustomFilter();

    QStandardItem* processAndCreate( const QString& line );
};

#endif
