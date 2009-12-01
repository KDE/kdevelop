/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

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
#include <QtGui/QAction>
#include <interfaces/idocumentation.h>

class QModelIndex;
class QWebView;
class QtHelpPlugin;

class QtHelpDocumentation : public QObject, public KDevelop::IDocumentation
{
    Q_OBJECT
    public:
        QtHelpDocumentation(const QString& name, const QMap<QString, QUrl>& info);
            
        QtHelpDocumentation(const QString& name, const QMap<QString, QUrl>& info, const QString& key);

        virtual QString name() const { return m_name; }
        
        virtual QString description() const;
        
        virtual QWidget* documentationWidget(QWidget* parent);
        
        virtual KDevelop::IDocumentationProvider* provider() const;
        QMap<QString, QUrl> info() const { return m_info; }
        
        static QtHelpPlugin* s_provider;
    private slots:
        void jumpedTo(const QUrl& newUrl);
        
    private:
        const QString m_name;
        const QMap<QString, QUrl> m_info;
        const QMap<QString, QUrl>::const_iterator m_current;
        QWebView* lastView;
};

class HomeDocumentation : public QObject, public KDevelop::IDocumentation
{
    Q_OBJECT
    public:
        virtual QWidget* documentationWidget(QWidget* parent = 0);
        virtual QString description() const { return QString(); }
        virtual QString name() const;
        virtual KDevelop::IDocumentationProvider* provider() const;
        
    public slots:
        void clicked(const QModelIndex& idx);
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