/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpprovider.h"

#include <QIcon>

QtHelpProvider::QtHelpProvider(QObject *parent, const QString &fileName, const QString &name, const QString &iconName, const QVariantList &args)
    : QtHelpProviderAbstract(parent, QHelpEngineCore::namespaceName(fileName) + QLatin1String(".qhc"), args)
    , m_fileName(fileName)
    , m_name(name)
    , m_iconName(iconName)
{
    m_engine.registerDocumentation(m_fileName);
}

QIcon QtHelpProvider::icon() const
{
    return QIcon::fromTheme(m_iconName);
}

QString QtHelpProvider::name() const
{
    return m_name;
}

QString QtHelpProvider::fileName() const
{
    return m_fileName;
}

QString QtHelpProvider::iconName() const
{
    return m_iconName;
}

void QtHelpProvider::setName(const QString& name)
{
    m_name = name;
}

void QtHelpProvider::setIconName(const QString& iconName)
{
    m_iconName = iconName;
}
