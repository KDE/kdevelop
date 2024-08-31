/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "environmentprofilelist.h"
#include "kdevstringhandler.h"
#include "debug.h"

#include <QMap>
#include <QStringList>
#include <QString>

#include <KConfigGroup>

#include <QProcessEnvironment>

namespace KDevelop {
class EnvironmentProfileListPrivate
{
public:
    QMap<QString, QMap<QString, QString>> m_profiles;
    QString m_defaultProfileName;
};
}

using namespace KDevelop;

namespace {

namespace Strings {
// TODO: migrate to more consistent key term "Default Environment Profile"
inline QString defaultEnvProfileKey() { return QStringLiteral("Default Environment Group"); }
inline QString envGroup() { return QStringLiteral("Environment Settings"); }
// TODO: migrate to more consistent key term "Profile List"
inline QString profileListKey() { return QStringLiteral("Group List"); }
inline QString defaultProfileName() { return QStringLiteral("default"); }
}

void decode(KConfig* config, EnvironmentProfileListPrivate* d)
{
    KConfigGroup cfg(config, Strings::envGroup());
    d->m_defaultProfileName = cfg.readEntry(Strings::defaultEnvProfileKey(), Strings::defaultProfileName());
    const QStringList profileNames =
        cfg.readEntry(Strings::profileListKey(), QStringList{Strings::defaultProfileName()});
    for (const auto& profileName : profileNames) {
        KConfigGroup envgrp(&cfg, profileName);
        QMap<QString, QString> variables;
        const auto varNames = envgrp.keyList();
        for (const QString& varname : varNames) {
            variables[varname] = envgrp.readEntry(varname, QString());
        }

        d->m_profiles.insert(profileName, variables);
    }
}

void encode(KConfig* config, const EnvironmentProfileListPrivate* d)
{
    KConfigGroup cfg(config, Strings::envGroup());
    cfg.writeEntry(Strings::defaultEnvProfileKey(), d->m_defaultProfileName);
    cfg.writeEntry(Strings::profileListKey(), d->m_profiles.keys());
    const auto oldGroupList = cfg.groupList();
    for (const QString& group : oldGroupList) {
        if (!d->m_profiles.contains(group)) {
            cfg.deleteGroup(group);
        }
    }

    for (auto it = d->m_profiles.cbegin(), itEnd = d->m_profiles.cend(); it != itEnd; ++it) {
        KConfigGroup envgrp(&cfg, it.key());
        envgrp.deleteGroup();

        const auto val = it.value();
        for (auto it2 = val.cbegin(), it2End = val.cend(); it2 != it2End; ++it2) {
            envgrp.writeEntry(it2.key(), *it2);
        }
    }

    cfg.sync();
}

}

EnvironmentProfileList::EnvironmentProfileList(const EnvironmentProfileList& rhs)
    : d_ptr(new EnvironmentProfileListPrivate(*rhs.d_ptr))
{
}

EnvironmentProfileList& EnvironmentProfileList::operator=(const EnvironmentProfileList& rhs)
{
    Q_D(EnvironmentProfileList);

    *d = *rhs.d_ptr;
    return *this;
}

EnvironmentProfileList::EnvironmentProfileList(const KSharedConfigPtr& config)
    : d_ptr(new EnvironmentProfileListPrivate)
{
    Q_D(EnvironmentProfileList);

    decode(config.data(), d);
}

EnvironmentProfileList::EnvironmentProfileList(KConfig* config)
    : d_ptr(new EnvironmentProfileListPrivate)
{
    Q_D(EnvironmentProfileList);

    decode(config, d);
}

EnvironmentProfileList::~EnvironmentProfileList() = default;

QMap<QString, QString> EnvironmentProfileList::variables(const QString& profileName) const
{
    Q_D(const EnvironmentProfileList);

    return d->m_profiles.value(profileName.isEmpty() ? d->m_defaultProfileName : profileName);
}

QMap<QString, QString>& EnvironmentProfileList::variables(const QString& profileName)
{
    Q_D(EnvironmentProfileList);

    return d->m_profiles[profileName.isEmpty() ? d->m_defaultProfileName : profileName];
}

QString EnvironmentProfileList::defaultProfileName() const
{
    Q_D(const EnvironmentProfileList);

    return d->m_defaultProfileName;
}

void EnvironmentProfileList::setDefaultProfile(const QString& profileName)
{
    Q_D(EnvironmentProfileList);

    if (profileName.isEmpty() ||
        !d->m_profiles.contains(profileName)) {
        return;
    }

    d->m_defaultProfileName = profileName;
}

void EnvironmentProfileList::saveSettings(KConfig* config) const
{
    Q_D(const EnvironmentProfileList);

    encode(config, d);
    config->sync();
}

void EnvironmentProfileList::loadSettings(KConfig* config)
{
    Q_D(EnvironmentProfileList);

    d->m_profiles.clear();
    decode(config, d);
}

QStringList EnvironmentProfileList::profileNames() const
{
    Q_D(const EnvironmentProfileList);

    return d->m_profiles.keys();
}

void EnvironmentProfileList::removeProfile(const QString& profileName)
{
    Q_D(EnvironmentProfileList);

    d->m_profiles.remove(profileName);
}

EnvironmentProfileList::EnvironmentProfileList()
    : d_ptr(new EnvironmentProfileListPrivate)
{
}

QStringList EnvironmentProfileList::createEnvironment(const QString& profileName,
                                                      const QStringList& defaultEnvironment) const
{
    QMap<QString, QString> retMap;
    for (const QString& line : defaultEnvironment) {
        QString varName = line.section(QLatin1Char('='), 0, 0);
        QString varValue = line.section(QLatin1Char('='), 1);
        retMap.insert(varName, varValue);
    }

    if (!profileName.isEmpty()) {
        const auto userMap = variables(profileName);

        for (QMap<QString, QString>::const_iterator it = userMap.constBegin();
             it != userMap.constEnd(); ++it) {
            retMap.insert(it.key(), it.value());
        }
    }

    QStringList env;
    env.reserve(retMap.size());
    for (QMap<QString, QString>::const_iterator it = retMap.constBegin();
         it != retMap.constEnd(); ++it) {
        env << it.key() + QLatin1Char('=') + it.value();
    }

    return env;
}

static QString expandVariable(const QString &key, const QString &value,
                              QMap<QString, QString> &output,
                              const QMap<QString, QString> &input,
                              const QProcessEnvironment &environment)
{
    if (value.isEmpty())
        return QString();

    auto it = output.constFind(key);
    if (it != output.constEnd()) {
        // nothing to do, value was expanded already
        return *it;
    }

    // not yet expanded, do that now

    auto variableValue = [&](const QString &variable) {
        if (environment.contains(variable)) {
            return environment.value(variable);
        } else if (variable == key) {
            // This must be a reference to a system environment variable of the
            // same name, which happens to be unset. Treat it as empty.
            return QString();
        } else if (input.contains(variable)) {
            return expandVariable(variable, input.value(variable), output, input, environment);
        } else {
            qCWarning(UTIL) << "Couldn't find replacement for" << variable;
            return QString();
        }
    };

    constexpr ushort escapeChar{'\\'};
    constexpr ushort variableStartChar{'$'};
    const auto isSpecialSymbol = [=](QChar c) {
        return c.unicode() == escapeChar || c.unicode() == variableStartChar;
    };

    auto& expanded = output[key];
    expanded.reserve(value.size());
    const int lastIndex = value.size() - 1;
    int i = 0;
    // Never treat value.back() as a special symbol (nothing to escape or start).
    while (i < lastIndex) {
        const auto currentChar = value[i];
        switch (currentChar.unicode()) {
        case escapeChar: {
            const auto nextChar = value[i+1];
            if (!isSpecialSymbol(nextChar)) {
                expanded += currentChar; // Nothing to escape => keep the escapeChar.
            }
            expanded += nextChar;
            i += 2;
            break;
        }
        case variableStartChar: {
            ++i;
            const auto match = matchPossiblyBracedAsciiVariable(QStringView{value}.sliced(i));
            if (match.length == 0) {
                expanded += currentChar; // Not a variable name start.
            } else {
                expanded += variableValue(match.name);
                i += match.length;
            }
            break;
        }
        default:
            expanded += currentChar;
            ++i;
        }
    }
    if (i == lastIndex) {
        expanded += value[i];
    }
    return expanded;
}

void KDevelop::expandVariables(QMap<QString, QString>& variables, const QProcessEnvironment& environment)
{
    QMap<QString, QString> expanded;
    for (auto it = variables.cbegin(), end = variables.cend(); it != end; ++it) {
        expandVariable(it.key(), it.value(), expanded, variables, environment);
    }
    variables = expanded;
}
