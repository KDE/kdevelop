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
        QString name() const override { return m_name; }
        QString description() const override;
        virtual bool providesWidget() const;
        QWidget* documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = nullptr) override;
        KDevelop::IDocumentationProvider* provider() const override;
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
        KDevelop::IDocumentationProvider* provider() const override;
        QString name() const override;
        QString description() const override { return name(); }
        QWidget* documentationWidget ( KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = nullptr ) override;
};


#endif // MANPAGEDOCUMENTATION_H
