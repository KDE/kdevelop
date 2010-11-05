/*  This file is part of KDevelop
    Copyright 2010 Yannick Motta <yannick.motta@gmail.com>

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

#include <interfaces/idocumentation.h>

#include <KUrl>

#include <QtCore/QObject>
#include <QtCore/QString>

class QWidget;
class QStackedWidget;
class KHTMLPart;

class ManPagePlugin;

class ManPageDocumentation : public QObject, public KDevelop::IDocumentation
{
    Q_OBJECT
    public:
        ManPageDocumentation(const KUrl& url, const QString& name, const QByteArray& description, ManPagePlugin* parent);

        virtual QString name() const { return m_name; }
        virtual QString description() const;
        virtual bool providesWidget() const;
        virtual QWidget* documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = 0);
        virtual KDevelop::IDocumentationProvider* provider() const;

      private:
        const KUrl m_url;
        const QString m_name;
        const QByteArray m_description;
        ManPagePlugin* m_parent;
        
};


#endif // MANPAGEDOCUMENTATION_H
