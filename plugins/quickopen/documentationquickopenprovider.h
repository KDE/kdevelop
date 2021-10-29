/*
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_DOCUMENTATIONQUICKOPENPROVIDER_H
#define KDEVPLATFORM_PLUGIN_DOCUMENTATIONQUICKOPENPROVIDER_H

#include <language/interfaces/quickopendataprovider.h>
#include <QVector>

class DocumentationQuickOpenProvider
    : public KDevelop::QuickOpenDataProviderBase
{
    Q_OBJECT
public:
    DocumentationQuickOpenProvider();

    KDevelop::QuickOpenDataPointer data(uint row) const override;
    uint unfilteredItemCount() const override;
    uint itemCount() const override;
    void reset() override;
    void setFilterText(const QString& text) override;
private:
    QVector<KDevelop::QuickOpenDataPointer> m_results;
};

#endif // KDEVPLATFORM_PLUGIN_DOCUMENTATIONQUICKOPENPROVIDER_H
