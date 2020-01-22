/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "kdevelopdashboarddeclarativeplugin.h"

#include "debug.h"
#include "icoreobject.h"

#include <qqml.h>
#include <vcs/models/brancheslistmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(const QLoggingCategory &category, QObject *parent = nullptr)
        : QObject(parent)
        , m_category(category)
    {}

    Q_INVOKABLE void log(const QString &message)
    {
        qCDebug(m_category) << message;
    }

private:
    const QLoggingCategory &m_category;
};

void KDevplatformDeclarativePlugin::registerTypes(const char* uri)
{
    static const QLoggingCategory loggingCategory{"kdevelop.plugins.welcomepage"};

    qmlRegisterSingletonType<KDevelop::ICore>(uri, 1, 0, "ICore", [](QQmlEngine*, QJSEngine*) -> QObject* {
        return KDevelop::ICore::self();
    });
    qmlRegisterSingletonType<KDevelop::ICore>(uri, 1, 0, "Logger", [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new Logger(loggingCategory);
    });
    qmlRegisterType<KDevelop::BranchesListModel>(uri, 1, 0, "BranchesListModel");

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterAnonymousType<KDevelop::IProject>(uri, 1);
#else
    qmlRegisterType<KDevelop::IProject>();
#endif
}

#include "kdevelopdashboarddeclarativeplugin.moc"
