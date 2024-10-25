/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPPROVIDER_H
#define QTHELPPROVIDER_H

#include "qthelpproviderabstract.h"

class QtHelpProvider : public QtHelpProviderAbstract
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IDocumentationProvider)
public:
    struct DocumentationFileInfo
    {
        QString filePath; ///< the absolute path to a .qch documentation file
        QString namespaceName; ///< the namespace name for @a filePath
    };

    /**
     * Constructor.
     */
    explicit QtHelpProvider(DocumentationFileInfo documentationFileInfo, const QString& name, const QString& iconName,
                            QObject* parent);

    QIcon icon() const override;
    QString name() const override;
    void setName(const QString& name);
    QString iconName() const;
    void setIconName(const QString& iconName);

    /**
     * @return the absolute file path to the source .qch documentation file
     *
     * @note the absolute file path to the .qhc collection file can be obtained via engine()->collectionFile().
     */
    [[nodiscard]] const QString& documentationFilePath() const
    {
        return m_documentationFileInfo.filePath;
    }
    /**
     * @return the namespace name of both the .qch documention file and the .qhc collection file
     */
    [[nodiscard]] const QString& namespaceName() const
    {
        return m_documentationFileInfo.namespaceName;
    }

private:
    const DocumentationFileInfo m_documentationFileInfo;
    QString m_name;
    QString m_iconName;
};

#endif // QTHELPPROVIDER_H
