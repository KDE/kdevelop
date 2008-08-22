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
class MakeOutputModel;
class QString;
// template <typename T1> class QList;
#include <QList>
class ErrorFormat;
class ActionFormat;
class QStandardItem;

class ErrorFilter
{
public:
    explicit ErrorFilter( MakeOutputModel const& model );
    ~ErrorFilter();

    QStandardItem* processAndCreate( const QString& line );

private:
    MakeOutputModel const& m_model;
    QList<ErrorFormat> m_errList;
};

class MakeActionFilter
{
public:
    explicit MakeActionFilter( MakeOutputModel& model );
    ~MakeActionFilter();

    QStandardItem* processAndCreate( const QString& line );

private:
    MakeOutputModel& m_model;
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
