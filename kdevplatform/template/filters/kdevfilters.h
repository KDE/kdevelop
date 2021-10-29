/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_KDEVFILTERS_H
#define KDEVPLATFORM_KDEVFILTERS_H

#include <grantlee/taglibraryinterface.h>
#include <grantlee/filter.h>

#include <QObject>

namespace KDevelop {

class CamelCaseFilter : public Grantlee::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class LowerCamelCaseFilter : public Grantlee::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class UnderscoreFilter : public Grantlee::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class UpperFirstFilter : public Grantlee::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class SplitLinesFilter : public Grantlee::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class ArgumentTypeFilter : public Grantlee::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class KDevFilters : public QObject, public Grantlee::TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(Grantlee::TagLibraryInterface)
    Q_PLUGIN_METADATA(IID "org.grantlee.TagLibraryInterface")

public:
    explicit KDevFilters(QObject* parent = nullptr, const QVariantList &args = QVariantList());
    ~KDevFilters() override;

    QHash< QString, Grantlee::Filter* > filters(const QString& name = QString()) override;
};

}

#endif // KDEVPLATFORM_KDEVFILTERS_H
