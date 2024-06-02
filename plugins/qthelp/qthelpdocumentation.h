/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPDOCUMENTATION_H
#define QTHELPDOCUMENTATION_H

#include <QList>
#include <QMap>
#include <QUrl>
#include <QAction>
#include <QHelpLink>

#include <interfaces/idocumentation.h>

namespace KDevelop { class StandardDocumentationView; }
class QModelIndex;
class QNetworkAccessManager;
class QtHelpProviderAbstract;

class QtHelpDocumentation : public KDevelop::IDocumentation
{
    Q_OBJECT
public:
    QtHelpDocumentation(QtHelpProviderAbstract* provider, const QString& name, const QList<QHelpLink>& info);
    QtHelpDocumentation(QtHelpProviderAbstract* provider, const QString& name, const QList<QHelpLink>& info,
                        const QString& key);

    QtHelpProviderAbstract* qtHelpProvider() const
    {
        return m_provider;
    }

    QString name() const override
    {
        return m_name;
    }

    QString description() const override;

    QWidget* documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent) override;

    KDevelop::IDocumentationProvider* provider() const override;

    QList<QHelpLink> info() const
    {
        return m_info;
    }

    const QUrl& currentUrl() const
    {
        return m_current->url;
    }
    const QString& currentTitle() const
    {
        return m_current->title;
    }

public Q_SLOTS:
    void viewContextMenuRequested(const QPoint& pos);

private Q_SLOTS:
    void jumpedTo(const QUrl& newUrl);

private:
    QtHelpProviderAbstract* m_provider;
    const QString m_name;
    const QList<QHelpLink> m_info;
    const QList<QHelpLink>::const_iterator m_current;

    KDevelop::StandardDocumentationView* lastView;
};

class HomeDocumentation : public KDevelop::IDocumentation
{
    Q_OBJECT
public:
    HomeDocumentation(QtHelpProviderAbstract* provider);
    QWidget* documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = nullptr) override;
    QString description() const override
    {
        return QString();
    }
    QString name() const override;
    KDevelop::IDocumentationProvider* provider() const override;

public Q_SLOTS:
    void clicked(const QModelIndex& idx);

private:
    QtHelpProviderAbstract* m_provider;
    bool eventFilter(QObject* obj, QEvent* event) override;
};

class QtHelpAlternativeLink : public QAction
{
    Q_OBJECT
public:
    QtHelpAlternativeLink(const QString& name, const QtHelpDocumentation* doc, QObject* parent);

public Q_SLOTS:
    void showUrl();

private:
    const QtHelpDocumentation* mDoc;
    const QString mName;
};

#endif
