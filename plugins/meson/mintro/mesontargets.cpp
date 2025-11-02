/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesontargets.h"

#include <debug.h>

#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>

using namespace std;
using namespace KDevelop;

// MesonTargetSources

MesonTargetSources::MesonTargetSources(const QJsonObject& json, MesonTarget* target)
    : m_target(target)
{
    fromJSON(json);
}

MesonTargetSources::~MesonTargetSources() {}

QString MesonTargetSources::language() const
{
    return m_language;
}

QStringList MesonTargetSources::compiler() const
{
    return m_compiler;
}

QStringList MesonTargetSources::parameters() const
{
    return m_parameters;
}

KDevelop::Path::List MesonTargetSources::sources() const
{
    return m_sources;
}

KDevelop::Path::List MesonTargetSources::generatedSources() const
{
    return m_generatedSources;
}

KDevelop::Path::List MesonTargetSources::allSources() const
{
    return m_sources + m_generatedSources;
}

KDevelop::Path::List MesonTargetSources::includeDirs() const
{
    return m_includeDirs;
}

QHash<QString, QString> MesonTargetSources::defines() const
{
    return m_defines;
}

QStringList MesonTargetSources::extraArgs() const
{
    return m_extraArgs;
}

MesonTarget* MesonTargetSources::target()
{
    return m_target;
}

void MesonTargetSources::fromJSON(const QJsonObject& json)
{
    m_language = json[QStringLiteral("language")].toString();

    QJsonArray comp = json[QStringLiteral("compiler")].toArray();
    QJsonArray param = json[QStringLiteral("parameters")].toArray();
    QJsonArray src = json[QStringLiteral("sources")].toArray();
    QJsonArray gensrc = json[QStringLiteral("generated_sources")].toArray();

    transform(begin(comp), end(comp), back_inserter(m_compiler), [](const auto& x) { return x.toString(); });
    transform(begin(param), end(param), back_inserter(m_parameters), [](const auto& x) { return x.toString(); });
    transform(begin(src), end(src), back_inserter(m_sources), [](const auto& x) { return Path(x.toString()); });
    transform(begin(gensrc), end(gensrc), back_inserter(m_generatedSources),
              [](const auto& x) { return Path(x.toString()); });

    splitParameters();
    qCDebug(KDEV_Meson) << "    - language:" << m_language << "has" << m_sources.count() + m_generatedSources.count()
                        << "files with" << m_includeDirs.count() << "include directories and" << m_defines.count()
                        << "defines";
}

void MesonTargetSources::splitParameters()
{
    for (const QString& i : m_parameters) {
        [&]() {
            for (auto j : { QStringLiteral("-I"), QStringLiteral("/I"), QStringLiteral("-isystem") }) {
                if (i.startsWith(j)) {
                    m_includeDirs << Path(i.mid(j.size()));
                    return;
                }
            }

            for (auto j : { QStringLiteral("-D"), QStringLiteral("/D") }) {
                if (i.startsWith(j)) {
                    QString define = i.mid(j.size());
                    QString name = define;
                    QString value;

                    int equalPos = define.indexOf(QLatin1Char('='));
                    if (equalPos > 0) {
                        name = define.left(equalPos);
                        value = define.mid(equalPos + 1);
                    }

                    m_defines[name] = value;
                    return;
                }
            }

            m_extraArgs << i;
        }();
    }
}

// MesonTarget

MesonTarget::MesonTarget(const QJsonObject& json)
{
    fromJSON(json);
}

MesonTarget::~MesonTarget() {}

QString MesonTarget::name() const
{
    return m_name;
}

QString MesonTarget::type() const
{
    return m_type;
}

KDevelop::Path MesonTarget::definedIn() const
{
    return m_definedIn;
}

KDevelop::Path::List MesonTarget::filename() const
{
    return m_filename;
}

bool MesonTarget::buildByDefault() const
{
    return m_buildByDefault;
}

bool MesonTarget::installed() const
{
    return m_installed;
}

QVector<MesonSourcePtr> MesonTarget::targetSources()
{
    return m_targetSources;
}

void MesonTarget::fromJSON(const QJsonObject& json)
{
    m_name = json[QStringLiteral("name")].toString();
    m_type = json[QStringLiteral("type")].toString();
    m_definedIn = Path(json[QStringLiteral("defined_in")].toString());
    m_buildByDefault = json[QStringLiteral("build_by_default")].toBool();
    m_installed = json[QStringLiteral("installed")].toBool();

    QJsonArray files = json[QStringLiteral("filename")].toArray();
    transform(begin(files), end(files), back_inserter(m_filename), [](const auto& x) { return Path(x.toString()); });

    qCDebug(KDEV_Meson) << "  - " << m_type << m_name;

    for (const auto& i : json[QStringLiteral("target_sources")].toArray()) {
        m_targetSources << make_shared<MesonTargetSources>(i.toObject(), this);
    }
}

// MesonTargets

MesonTargets::MesonTargets(const QJsonArray& json)
{
    fromJSON(json);
}

MesonTargets::~MesonTargets() {}

QVector<MesonTargetPtr> MesonTargets::targets()
{
    return m_targets;
}

MesonSourcePtr MesonTargets::fileSource(KDevelop::Path p)
{
    auto it = m_sourceHash.find(p);
    if (it == end(m_sourceHash)) {
        return nullptr;
    }

    return *it;
}

MesonSourcePtr MesonTargets::operator[](KDevelop::Path p)
{
    return fileSource(p);
}

void MesonTargets::fromJSON(const QJsonArray& json)
{
    qCDebug(KDEV_Meson) << "MINTRO: Loading targets from json...";
    for (const auto& i : json) {
        m_targets << make_shared<MesonTarget>(i.toObject());
    }

    buildHashMap();
    qCDebug(KDEV_Meson) << "MINTRO: Loaded" << m_targets.count() << "targets with" << m_sourceHash.count()
                        << "total files";
}

void MesonTargets::buildHashMap()
{
    for (auto& i : m_targets) {
        for (auto j : i->targetSources()) {
            for (auto k : j->allSources()) {
                m_sourceHash[k] = j;
            }
        }
    }
}
