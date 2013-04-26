/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_KDEVFILTERS_H
#define KDEVPLATFORM_KDEVFILTERS_H

#include <grantlee/taglibraryinterface.h>
#include <grantlee/filter.h>

#include <QObject>
#include <QtPlugin>

namespace KDevelop {

class CamelCaseFilter : public Grantlee::Filter
{
public:
    virtual QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const;
};

class LowerCamelCaseFilter : public Grantlee::Filter
{
public:
    virtual QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const;
};

class UnderscoreFilter : public Grantlee::Filter
{
public:
    virtual QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const;
};

class UpperFirstFilter : public Grantlee::Filter
{
public:
    virtual QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const;
};

class SplitLinesFilter : public Grantlee::Filter
{
public:
    virtual QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const;
};

class ArgumentTypeFilter : public Grantlee::Filter
{
public:
    virtual QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const;
};

class KDevFilters : public QObject, public Grantlee::TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(Grantlee::TagLibraryInterface)

public:
    explicit KDevFilters(QObject* parent = 0);
    virtual ~KDevFilters();

    virtual QHash< QString, Grantlee::Filter* > filters(const QString& name = QString());
};

}

#endif // KDEVPLATFORM_KDEVFILTERS_H
