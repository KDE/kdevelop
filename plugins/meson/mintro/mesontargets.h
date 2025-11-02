/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    explicit MesonTargetSources(const QJsonObject& json, MesonTarget* target);
    virtual ~MesonTargetSources();

    QString language() const;
    QStringList compiler() const;
    QStringList parameters() const;
    KDevelop::Path::List sources() const;
    KDevelop::Path::List generatedSources() const;
    KDevelop::Path::List allSources() const;

    KDevelop::Path::List includeDirs() const;
    QHash<QString, QString> defines() const;
    QStringList extraArgs() const;

    MesonTarget* target();

    void fromJSON(const QJsonObject& json);

private:
    QString m_language;
    QStringList m_compiler;
    QStringList m_parameters;
    KDevelop::Path::List m_sources;
    KDevelop::Path::List m_generatedSources;

    KDevelop::Path::List m_includeDirs;
    QHash<QString, QString> m_defines;
    QStringList m_extraArgs;

    MesonTarget* m_target; // Store a pointer to the parent target

    void splitParameters();
};

class MesonTarget
{
public:
    explicit MesonTarget(const QJsonObject& json);
    virtual ~MesonTarget();

    QString name() const;
    QString type() const;
    KDevelop::Path definedIn() const;
    KDevelop::Path::List filename() const;
    bool buildByDefault() const;
    bool installed() const;

    QVector<MesonSourcePtr> targetSources();

    void fromJSON(const QJsonObject& json);

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
    explicit MesonTargets(const QJsonArray& json);
    virtual ~MesonTargets();

    QVector<MesonTargetPtr> targets();

    MesonSourcePtr fileSource(KDevelop::Path p);
    MesonSourcePtr operator[](KDevelop::Path p);

    void fromJSON(const QJsonArray& json);

private:
    QVector<MesonTargetPtr> m_targets;
    QHash<KDevelop::Path, MesonSourcePtr> m_sourceHash;

    void buildHashMap();
};
