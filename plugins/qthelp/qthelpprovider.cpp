/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpprovider.h"

#include "debug.h"

#include <QIcon>

QtHelpProvider::QtHelpProvider(QObject* parent, const QString& collectionFilePath, const QString& qchInputFileName,
                               const QString& nameSpace, const QString& name, const QString& iconName)
    : QtHelpProviderAbstract(parent, collectionFilePath)
    , m_qchInputFilePath(qchInputFileName)
    , m_nameSpace(nameSpace)
    , m_name(name)
    , m_iconName(iconName)
{
    Q_ASSERT(QHelpEngineCore::namespaceName(m_qchInputFilePath) == nameSpace);

    if (!engine()->registeredDocumentations().isEmpty()) {
        // data was already registered previously
        Q_ASSERT(engine()->registeredDocumentations().size() == 1);
        Q_ASSERT(engine()->documentationFileName(nameSpace) == m_qchInputFilePath);
        return;
    }

    if (!engine()->registerDocumentation(m_qchInputFilePath)) {
        qCCritical(QTHELP) << "error >> " << m_qchInputFilePath << engine()->error();
    }
}

QIcon QtHelpProvider::icon() const
{
    return QIcon::fromTheme(m_iconName);
}

QString QtHelpProvider::name() const
{
    return m_name;
}

QString QtHelpProvider::qchInputFilePath() const
{
    return m_qchInputFilePath;
}

QString QtHelpProvider::nameSpace() const
{
    return m_nameSpace;
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

#include "moc_qthelpprovider.cpp"
