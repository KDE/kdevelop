/*
    SPDX-FileCopyrightText: 2012 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

    qmlRegisterAnonymousType<KDevelop::IProject>(uri, 1);
}

#include "kdevelopdashboarddeclarativeplugin.moc"
