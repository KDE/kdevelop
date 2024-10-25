/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpprovider.h"

#include <QIcon>

QtHelpProvider::QtHelpProvider(DocumentationFileInfo documentationFileInfo, const QString& name,
                               const QString& iconName, QObject* parent)
    : QtHelpProviderAbstract(parent, documentationFileInfo.namespaceName + QLatin1String(".qhc"))
    , m_documentationFileInfo(std::move(documentationFileInfo))
    , m_name(name)
    , m_iconName(iconName)
{
    Q_ASSERT(QHelpEngineCore::namespaceName(m_documentationFileInfo.filePath) == m_documentationFileInfo.namespaceName);

    bool registerDocumentationFile = true;
    cleanUpRegisteredDocumentations([&registerDocumentationFile, this](const QString& namespaceName) {
        if (!registerDocumentationFile) {
            // Unregister this namespace, because m_documentationFileInfo's namespace has already been found.
            return true;
        }

        if (namespaceName != m_documentationFileInfo.namespaceName) {
            return true; // unregister this unneeded namespace
        }
        const auto filePath = m_engine.documentationFileName(namespaceName);
        if (filePath != m_documentationFileInfo.filePath) {
            return true; // unregister this namespace associated with an unneeded .qch file
        }

        // The .qch file specified by m_documentationFileInfo is already registered and up-to-date.
        registerDocumentationFile = false; // do not reregister it
        return false; // keep its namespace registered with the engine
    });

    if (registerDocumentationFile) {
        registerDocumentation(m_documentationFileInfo.filePath);
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
