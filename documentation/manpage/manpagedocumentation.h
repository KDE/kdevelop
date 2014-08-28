/*  This file is part of KDevelop
    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>
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

#ifndef MANPAGEDOCUMENTATION_H
#define MANPAGEDOCUMENTATION_H

#include "manpagemodel.h"

#include <interfaces/idocumentation.h>

#include <QUrl>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QIODevice>

class QWidget;
class QStackedWidget;
class QTreeView;

class ManPagePlugin;

class ManPageDocumentation : public KDevelop::IDocumentation
{
    Q_OBJECT
    public:
        ManPageDocumentation(const QString& name, const QUrl& url);
        virtual QString name() const { return m_name; }
        virtual QString description() const;
        virtual bool providesWidget() const;
        virtual QWidget* documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = 0);
        virtual KDevelop::IDocumentationProvider* provider() const;
        static ManPagePlugin* s_provider;

    private slots:
        void finished(KJob*);

    private:
        const QUrl m_url;
        const QString m_name;
        QString m_description;

};

class ManPageHomeDocumentation : public KDevelop::IDocumentation
{
    Q_OBJECT
    public:
        virtual KDevelop::IDocumentationProvider* provider() const;
        virtual QString name() const;
        virtual QString description() const { return name(); }
        virtual QWidget* documentationWidget ( KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = 0 );
};


#endif // MANPAGEDOCUMENTATION_H
