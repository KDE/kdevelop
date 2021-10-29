/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesonkwargsinfo.h"
#include "debug.h"

#include <QJsonArray>

MesonKWARGSInfo::MesonKWARGSInfo(MesonKWARGSInfo::Function fn, const QString& id)
    : m_func(fn)
    , m_id(id)
{
}

QJsonObject MesonKWARGSInfo::command()
{
    QJsonObject res;

    auto func2str = [](Function fn) -> QString {
        switch (fn) {
        case PROJECT:
            return QStringLiteral("project");
        case TARGET:
            return QStringLiteral("target");
        case DEPENDENCY:
            return QStringLiteral("dependency");
        }

        return QStringLiteral("ERROR");
    };

    res[QStringLiteral("type")] = QStringLiteral("kwargs");
    res[QStringLiteral("function")] = func2str(m_func);
    res[QStringLiteral("id")] = m_id;
    res[QStringLiteral("operation")] = QStringLiteral("info");

    m_infoID = func2str(m_func) + QLatin1Char('#') + m_id;

    return res;
}

void MesonKWARGSInfo::parseResult(QJsonObject data)
{
    if (!data[QStringLiteral("kwargs")].isObject()) {
        qCWarning(KDEV_Meson) << "REWRITR: Failed to parse rewriter result";
        return;
    }

    QJsonObject kwargs = data[QStringLiteral("kwargs")].toObject();

    if (!kwargs[m_infoID].isObject()) {
        qCWarning(KDEV_Meson) << "REWRITR: Failed to extract info data from object";
        return;
    }

    m_result = kwargs[m_infoID].toObject();
}

MesonKWARGSInfo::Function MesonKWARGSInfo::function() const
{
    return m_func;
}

QString MesonKWARGSInfo::id() const
{
    return m_id;
}

bool MesonKWARGSInfo::hasKWARG(const QString& kwarg) const
{
    return m_result.contains(kwarg);
}

QJsonValue MesonKWARGSInfo::get(const QString& kwarg) const
{
    if (!hasKWARG(kwarg)) {
        return QJsonValue();
    }
    return m_result[kwarg];
}

QString MesonKWARGSInfo::getString(const QString& kwarg) const
{
    return get(kwarg).toString();
}

QStringList MesonKWARGSInfo::getArray(const QString& kwarg) const
{
    QStringList result;
    for (auto i : get(kwarg).toArray()) {
        result += i.toString();
    }
    return result;
}

// Constructors

MesonKWARGSProjectInfo::MesonKWARGSProjectInfo()
    : MesonKWARGSInfo(PROJECT, QStringLiteral("/"))
{
}
MesonKWARGSTargetInfo::MesonKWARGSTargetInfo(const QString& id)
    : MesonKWARGSInfo(TARGET, id)
{
}
MesonKWARGSDependencyInfo::MesonKWARGSDependencyInfo(const QString& id)
    : MesonKWARGSInfo(DEPENDENCY, id)
{
}

// Destructors

MesonKWARGSInfo::~MesonKWARGSInfo() {}
MesonKWARGSProjectInfo::~MesonKWARGSProjectInfo() {}
MesonKWARGSTargetInfo::~MesonKWARGSTargetInfo() {}
MesonKWARGSDependencyInfo::~MesonKWARGSDependencyInfo() {}
