/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdeprojectsmodel.h"
#include <QIcon>
#include <QUrl>
#include <QDebug>
#include <vcs/vcslocation.h>

using namespace KDevelop;

SourceItem::SourceItem(const Source& s)
    : QStandardItem(QIcon::fromTheme(s.icon), s.name), m_s(s)
{
    Q_ASSERT(!m_s.name.isEmpty());
}

QVariant SourceItem::data(int role) const
{
    switch(role)
    {
        case KDEProjectsModel::IdentifierRole:
            return m_s.identifier;
        case KDEProjectsModel::VcsLocationRole:
            return QVariantMap(m_s.m_urls);
        default:
            return QStandardItem::data(role);
    }
}

Source::Source(const SourceType& aType, const QString& anIcon, const QString& aName, const QPair<QString, QString>& aUrl)
    : type(aType), name(aName), icon(anIcon)
{
    m_urls[aUrl.first] = aUrl.second;
}

KDEProjectsModel::KDEProjectsModel(QObject* parent): QStandardItemModel(parent)
{}
