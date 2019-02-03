/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

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
    KJob* configure(KDevelop::IProject* project, Meson::BuildDir const& buildDir, QStringList args,
                    DirectoryStatus status = ___UNDEFINED___);

    /// Evaluate a directory for the use with meson
    static DirectoryStatus evaluateBuildDirectory(KDevelop::Path const& path, QString const& backend);

    bool hasError() const;
    QString errorDescription() const;

    QList<KDevelop::IProjectBuilder *> additionalBuilderPlugins(KDevelop::IProject * project) const override;

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

    KJob* configureIfRequired(KDevelop::IProject* project, KJob *realJob);
};
