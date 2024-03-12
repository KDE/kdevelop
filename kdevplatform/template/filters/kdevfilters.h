/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_KDEVFILTERS_H
#define KDEVPLATFORM_KDEVFILTERS_H

#include <KTextTemplate/Filter>
#include <KTextTemplate/TagLibraryInterface>

#include <QObject>

namespace KDevelop {

class CamelCaseFilter : public KTextTemplate::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class LowerCamelCaseFilter : public KTextTemplate::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class UnderscoreFilter : public KTextTemplate::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class UpperFirstFilter : public KTextTemplate::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class SplitLinesFilter : public KTextTemplate::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class ArgumentTypeFilter : public KTextTemplate::Filter
{
public:
    QVariant doFilter(const QVariant& input,
                              const QVariant& argument = QVariant(),
                              bool autoescape = false) const override;
};

class KDevFilters : public QObject, public KTextTemplate::TagLibraryInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextTemplate::TagLibraryInterface)
    Q_PLUGIN_METADATA(IID "org.kde.KTextTemplate.TagLibraryInterface")

public:
    explicit KDevFilters(QObject* parent = nullptr, const QVariantList &args = QVariantList());
    ~KDevFilters() override;

    QHash<QString, KTextTemplate::Filter*> filters(const QString& name = QString()) override;
};

}

#endif // KDEVPLATFORM_KDEVFILTERS_H
