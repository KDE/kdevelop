/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpprovider.h"

#include <QIcon>

QtHelpProvider::QtHelpProvider(QObject* parent, const QString& fileName, const QString& namespaceName,
                               const QString& name, const QString& iconName)
    : QtHelpProviderAbstract(parent, namespaceName + QLatin1String(".qhc"))
    , m_fileName(fileName)
    , m_namespaceName(namespaceName)
    , m_name(name)
    , m_iconName(iconName)
{
    Q_ASSERT(QHelpEngineCore::namespaceName(fileName) == namespaceName);

    bool registerFileName = true;
    cleanUpRegisteredDocumentations([&registerFileName, this](const QString& namespaceName) {
        if (!registerFileName) {
            // Unregister this namespace, because the namespace associated with m_fileName has already been found.
            return true;
        }

        if (namespaceName != m_namespaceName) {
            return true; // unregister this unneeded namespace
        }
        const auto filePath = m_engine.documentationFileName(namespaceName);
        if (filePath != m_fileName) {
            return true; // unregister this namespace associated with an unneeded .qch file
        }

        // The .qch file m_fileName is already registered and up-to-date.
        registerFileName = false; // do not reregister it
        return false; // keep its namespace registered with the engine
    });

    if (registerFileName) {
        registerDocumentation(m_fileName);
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

QString QtHelpProvider::fileName() const
{
    return m_fileName;
}

QString QtHelpProvider::namespaceName() const
{
    return m_namespaceName;
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
