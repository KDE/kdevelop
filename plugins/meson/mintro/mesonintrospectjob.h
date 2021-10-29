/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "mesonconfig.h"
#include "mesonoptions.h"
#include "mesonprojectinfo.h"
#include "mesontargets.h"
#include "mesontests.h"

#include <interfaces/iproject.h>

#include <KJob>

#include <QFutureWatcher>

#include <memory>

class QJsonObject;

class MesonIntrospectJob : public KJob
{
    Q_OBJECT

public:
    enum Type { BENCHMARKS, BUILDOPTIONS, BUILDSYSTEM_FILES, DEPENDENCIES, INSTALLED, PROJECTINFO, TARGETS, TESTS };
    enum Mode { BUILD_DIR, MESON_FILE };

public:
    explicit MesonIntrospectJob(KDevelop::IProject* project, QVector<Type> types, Mode mode, QObject* parent);
    explicit MesonIntrospectJob(KDevelop::IProject* project, KDevelop::Path meson, QVector<Type> types,
                                QObject* parent);
    explicit MesonIntrospectJob(KDevelop::IProject* project, Meson::BuildDir buildDir, QVector<Type> types, Mode mode,
                                QObject* parent);

    void start() override;
    bool doKill() override;

    QString getTypeString(Type type) const;

    MesonOptsPtr buildOptions();
    MesonProjectInfoPtr projectInfo();
    MesonTargetsPtr targets();
    MesonTestSuitesPtr tests();

private:
    QString importJSONFile(const Meson::BuildDir& buildDir, Type type, QJsonObject* out);
    QString importMesonAPI(const Meson::BuildDir& buildDir, Type type, QJsonObject* out);
    QString import(Meson::BuildDir buildDir);
    void finished();

    QFutureWatcher<QString> m_futureWatcher;

    // The commands to execute
    QVector<Type> m_types = {};
    Mode m_mode = BUILD_DIR;
    Meson::BuildDir m_buildDir;
    KDevelop::Path m_projectPath;
    KDevelop::IProject* m_project = nullptr;

    // The results
    MesonOptsPtr m_res_options = nullptr;
    MesonProjectInfoPtr m_res_projectInfo = nullptr;
    MesonTargetsPtr m_res_targets = nullptr;
    MesonTestSuitesPtr m_res_tests = nullptr;
};
