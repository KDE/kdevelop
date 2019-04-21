/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

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

#include "mesonconfig.h"
#include "mesonoptions.h"
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
    MesonTargetsPtr targets();
    MesonTestSuitesPtr tests();

private:
    QString importJSONFile(Meson::BuildDir const& buildDir, Type type, QJsonObject* out);
    QString importMesonAPI(Meson::BuildDir const& buildDir, Type type, QJsonObject* out);
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
    MesonTargetsPtr m_res_targets = nullptr;
    MesonTestSuitesPtr m_res_tests = nullptr;
};
