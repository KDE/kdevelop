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
    explicit QtHelpQtDoc(QObject* parent, const QString& qmake, const QString& collectionFileName);
    ~QtHelpQtDoc();

    QIcon icon() const override;
    QString name() const override;
    void registerDocumentations();
    void loadDocumentation();
    void unloadDocumentation();
    /** @return local paths to all QCH files found in QT_INSTALL_DOCS directory **/
    QStringList qchFiles() const;

    /** @return @c true once the async initialization has finished */
    bool isInitialized() const { return m_isInitialized; }

    /** @return available qmake binaries */
    static QStringList qmakeCandidates();

Q_SIGNALS:
    /** emitted when isInitialized() is set to @c true */
    void isInitializedChanged();

private:
    QString m_qmake;
    QString m_path;
    bool m_isInitialized = false;
};

#endif // QTHELPQTDOC_H
