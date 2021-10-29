/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <project/interfaces/iprojectbuilder.h>
#include <util/path.h>

namespace Meson
{
struct BuildDir;
}

class MesonBuilder : public QObject, public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IProjectBuilder)
public:
    enum DirectoryStatus {
        DOES_NOT_EXIST = 0,
        CLEAN,
        MESON_CONFIGURED,
        MESON_FAILED_CONFIGURATION,
        INVALID_BUILD_DIR,
        DIR_NOT_EMPTY,
        EMPTY_STRING,
        ___UNDEFINED___
    };

    explicit MesonBuilder(QObject* parent);

    KJob* build(KDevelop::ProjectBaseItem* item) override;
    KJob* clean(KDevelop::ProjectBaseItem* item) override;
    KJob* install(KDevelop::ProjectBaseItem* dom, const QUrl& installPath) override;
    KJob* prune(KDevelop::IProject* project) override;

    KJob* configure(KDevelop::IProject* project) override;
    KJob* configure(KDevelop::IProject* project, const Meson::BuildDir& buildDir, QStringList args,
                    DirectoryStatus status = ___UNDEFINED___);

    /// Evaluate a directory for the use with meson
    static DirectoryStatus evaluateBuildDirectory(const KDevelop::Path& path, const QString& backend);

    bool hasError() const;
    QString errorDescription() const;

    QList<KDevelop::IProjectBuilder*> additionalBuilderPlugins(KDevelop::IProject* project) const override;

Q_SIGNALS:
    void built(KDevelop::ProjectBaseItem*);
    void installed(KDevelop::ProjectBaseItem*);
    void cleaned(KDevelop::ProjectBaseItem*);
    void failed(KDevelop::ProjectBaseItem*);
    void configured(KDevelop::IProject*);
    void pruned(KDevelop::IProject*);

private:
    KDevelop::IProjectBuilder* m_ninjaBuilder = nullptr;
    QString m_errorString;

    KJob* configureIfRequired(KDevelop::IProject* project, KJob* realJob);
};
