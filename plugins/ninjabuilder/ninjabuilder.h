/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2017 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVNINJABUILDERPLUGIN_H
#define KDEVNINJABUILDERPLUGIN_H

#include "ninjajob.h"

#include <interfaces/iplugin.h>
#include <project/interfaces/iprojectbuilder.h>
#include <util/objectlist.h>
#include <QUrl>
#include <QVariantList>

class NinjaBuilder
    : public KDevelop::IPlugin
    , public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IProjectBuilder)

public:
    explicit NinjaBuilder(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args = QVariantList());

    KJob* build(KDevelop::ProjectBaseItem* item) override;
    KJob* clean(KDevelop::ProjectBaseItem* item) override;
    KJob* install(KDevelop::ProjectBaseItem* dom, const QUrl& installPath) override;
    KJob* install(KDevelop::ProjectBaseItem* item);

    NinjaJob* runNinja(KDevelop::ProjectBaseItem* item, NinjaJob::CommandType commandType,
                       const QStringList& args, const QByteArray& signal);

    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

Q_SIGNALS:
    void built(KDevelop::ProjectBaseItem* item);
    void failed(KDevelop::ProjectBaseItem* item);
    void installed(KDevelop::ProjectBaseItem* item);
    void cleaned(KDevelop::ProjectBaseItem* item);

private:
    KDevelop::ObjectList<NinjaJob> m_activeNinjaJobs;
};

#endif // KDEVNINJABUILDERPLUGIN_H
