/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPPROVIDERABSTRACT_H
#define QTHELPPROVIDERABSTRACT_H

#include <interfaces/idocumentationprovider.h>
#include <QObject>
#include <QHelpEngine>

class HelpNetworkAccessManager;

class QtHelpProviderAbstract : public QObject, public KDevelop::IDocumentationProvider
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IDocumentationProvider )
public:
    QtHelpProviderAbstract(QObject* parent, const QString& collectionFilePath);
    ~QtHelpProviderAbstract() override;

    KDevelop::IDocumentation::Ptr documentationForDeclaration (KDevelop::Declaration*) const override;
    KDevelop::IDocumentation::Ptr documentation(const QUrl& url) const override;

    KDevelop::IDocumentation::Ptr documentationForIndex(const QModelIndex& idx) const override;
    QAbstractItemModel* indexModel() const override;

    KDevelop::IDocumentation::Ptr homePage() const override;

    /// @return False in case we failed to load any documentation files, else true
    bool isValid() const;

    QHelpEngine* engine() { return &m_engine; }

    HelpNetworkAccessManager* networkAccess() const;
public Q_SLOTS:
    void jumpedTo(const QUrl& newUrl);
protected:
    QHelpEngine m_engine;
    HelpNetworkAccessManager* const m_nam;
};

#endif // QTHELPPROVIDERABSTRACT_H
