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

#include <util/path.h>

#include <QHash>
#include <QVector>

#include <memory>

class QJsonArray;
class QJsonObject;
class MesonTarget;
class MesonTargets;
class MesonTargetSources;

using MesonSourcePtr = std::shared_ptr<MesonTargetSources>;
using MesonTargetPtr = std::shared_ptr<MesonTarget>;
using MesonTargetsPtr = std::shared_ptr<MesonTargets>;

class MesonTargetSources
{
public:
    explicit MesonTargetSources(QJsonObject const& json, MesonTarget* target);
    virtual ~MesonTargetSources();

    QString language() const;
    QStringList compiler() const;
    QStringList paramerters() const;
    KDevelop::Path::List sources() const;
    KDevelop::Path::List generatedSources() const;
    KDevelop::Path::List allSources() const;

    KDevelop::Path::List includeDirs() const;
    QHash<QString, QString> defines() const;
    QStringList extraArgs() const;

    MesonTarget* target();

    void fromJSON(QJsonObject const& json);

private:
    QString m_language;
    QStringList m_compiler;
    QStringList m_paramerters;
    KDevelop::Path::List m_sources;
    KDevelop::Path::List m_generatedSources;

    KDevelop::Path::List m_includeDirs;
    QHash<QString, QString> m_defines;
    QStringList m_extraArgs;

    MesonTarget* m_target; // Store a pointer to the parent target

    void splitParamerters();
};

class MesonTarget
{
public:
    explicit MesonTarget(QJsonObject const& json);
    virtual ~MesonTarget();

    QString name() const;
    QString type() const;
    KDevelop::Path definedIn() const;
    KDevelop::Path::List filename() const;
    bool buildByDefault() const;
    bool installed() const;

    QVector<MesonSourcePtr> targetSources();

    void fromJSON(QJsonObject const& json);

private:
    QString m_name;
    QString m_type;
    KDevelop::Path m_definedIn;
    KDevelop::Path::List m_filename;
    bool m_buildByDefault;
    bool m_installed;

    QVector<MesonSourcePtr> m_targetSources;
};

class MesonTargets
{
public:
    explicit MesonTargets(QJsonArray const& json);
    virtual ~MesonTargets();

    QVector<MesonTargetPtr> targets();

    MesonSourcePtr fileSource(KDevelop::Path p);
    MesonSourcePtr operator[](KDevelop::Path p);

    void fromJSON(QJsonArray const& json);

private:
    QVector<MesonTargetPtr> m_targets;
    QHash<KDevelop::Path, MesonSourcePtr> m_sourceHash;

    void buildHashMap();
};
