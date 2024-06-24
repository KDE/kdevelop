/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>
    SPDX-FileCopyrightText: 2016 Andreas Cord-Landwehr <cordlandwehr@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPQTDOC_H
#define QTHELPQTDOC_H

#include "qthelpproviderabstract.h"

class QtHelpQtDoc : public QtHelpProviderAbstract
{
    Q_OBJECT
public:
    explicit QtHelpQtDoc(QObject* parent);
    ~QtHelpQtDoc();

    QIcon icon() const override;
    QString name() const override;
    void registerDocumentations();
    void loadDocumentation();
    void unloadDocumentation();
    /** @return local paths to all QCH files found in QT_INSTALL_DOCS directory **/
    QStringList qchFiles() const;

    /** @return true once the async initialization has finished */
    bool isInitialized() const { return m_isInitialized; }

Q_SIGNALS:
    /** emitted when @c isInitialized() is set to true */
    void isInitializedChanged();

private:
    QString m_path;
    bool m_isInitialized = false;
};

#endif // QTHELPQTDOC_H
