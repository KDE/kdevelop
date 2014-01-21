/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QTHELPDOCUMENTATION_H
#define QTHELPDOCUMENTATION_H

#include <QtCore/QMap>
#include <QtCore/QUrl>
#include <QAction>
#include <interfaces/idocumentation.h>

class QModelIndex;
class QWebView;
class QtHelpProviderAbstract;
class QTemporaryFile;

class QtHelpDocumentation : public KDevelop::IDocumentation
{
    Q_OBJECT
    public:
        QtHelpDocumentation(const QString& name, const QMap<QString, QUrl>& info);

        QtHelpDocumentation(const QString& name, const QMap<QString, QUrl>& info, const QString& key);

        virtual QString name() const { return m_name; }

        virtual QString description() const;

        virtual QWidget* documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent);

        virtual KDevelop::IDocumentationProvider* provider() const;
        QMap<QString, QUrl> info() const { return m_info; }

        static QtHelpProviderAbstract* s_provider;

    public slots:
        void viewContextMenuRequested(const QPoint& pos);

    private slots:
        void jumpedTo(const QUrl& newUrl);

    private:
        void setUserStyleSheet(QWebView* view, const QUrl& url);

    private:
        QtHelpProviderAbstract *m_provider;
        const QString m_name;
        const QMap<QString, QUrl> m_info;
        const QMap<QString, QUrl>::const_iterator m_current;
        QWebView* lastView;
        QWeakPointer<QTemporaryFile> m_lastStyleSheet;
};

class HomeDocumentation : public KDevelop::IDocumentation
{
    Q_OBJECT
    public:
        HomeDocumentation();
        virtual QWidget* documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = 0);
        virtual QString description() const { return QString(); }
        virtual QString name() const;
        virtual KDevelop::IDocumentationProvider* provider() const;

    public slots:
        void clicked(const QModelIndex& idx);
    private:
        QtHelpProviderAbstract *m_provider;
};

class QtHelpAlternativeLink : public QAction
{
    Q_OBJECT
    public:
        QtHelpAlternativeLink(const QString& name, const QtHelpDocumentation* doc, QObject* parent);

    public slots:
        void showUrl();

    private:
        const QtHelpDocumentation* mDoc;
        const QString mName;
};

#endif
