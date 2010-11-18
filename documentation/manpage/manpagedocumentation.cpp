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

#include <QLabel>
#include <KLocale>
#include <QTreeView>

#include "manpagedocumentation.h"
#include "manpageplugin.h"

#include <QtDebug>



ManPageDocumentation::ManPageDocumentation(const KUrl& url, const QString& name, const QByteArray& description, ManPagePlugin* parent)
    : m_url(url), m_name(name), m_description(description), m_parent(parent)
{
}

KDevelop::IDocumentationProvider* ManPageDocumentation::provider() const
{
    return m_parent;
}

QString ManPageDocumentation::description() const
{
    return QString::fromUtf8( m_description );
}

QWidget* ManPageDocumentation::documentationWidget(KDevelop::DocumentationFindWidget* findWidget, QWidget* parent )
{  

    QTreeView* contents=new QTreeView(parent);

    ManPageModel* model = new ManPageModel(contents);

    contents->setModel(model);

    QObject::connect(contents, SIGNAL(clicked(QModelIndex)), model, SLOT(showItem(QModelIndex)));
    return contents;
}


bool ManPageDocumentation::providesWidget() const
{
    return false;
}
