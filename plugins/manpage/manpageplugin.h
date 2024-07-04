/*
    SPDX-FileCopyrightText: 2010 Yannick Motta <yannick.motta@gmail.com>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MANPAGEPLUGIN_H
#define MANPAGEPLUGIN_H

#include "manpagemodel.h"

#include <interfaces/iplugin.h>
#include <interfaces/idocumentationprovider.h>

#include <QVariantList>
#include <QProgressBar>

class ManPageDocumentation;

class ManPagePlugin : public KDevelop::IPlugin, public KDevelop::IDocumentationProvider
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IDocumentationProvider )
public:
    explicit ManPagePlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args = QVariantList());
    ~ManPagePlugin() override;
    KDevelop::IDocumentation::Ptr documentationForDeclaration (KDevelop::Declaration* dec) const override;
    KDevelop::IDocumentation::Ptr documentation(const QUrl& url) const override;
    QAbstractItemModel* indexModel() const override;
    KDevelop::IDocumentation::Ptr documentationForIndex(const QModelIndex& index) const override;
    QIcon icon() const override;
    QString name() const override;
    KDevelop::IDocumentation::Ptr homePage() const override;
    void deleteProgressBar();

    ManPageModel* model() const;
    QProgressBar* progressBar() const;

private:
    KDevelop::IDocumentation::Ptr documentationForIdentifier(const QString& identifier) const;

    ManPageModel *m_model;
};

#endif // MANPAGEPLUGIN_H
