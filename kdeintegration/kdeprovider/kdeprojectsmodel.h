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

#ifndef KDEPROJECTSMODEL_H
#define KDEPROJECTSMODEL_H

#include <QStandardItemModel>

namespace KDevelop {
    class VcsLocation;
}

struct Source
{
    enum SourceType
    {
        Project,
        Module
    };
    
    Source() {}
    Source(const SourceType & aType, const QString& anIcon, const QString& aName, const QPair< QString, QString >& aUrl);
    
    SourceType type;
    QString name;
    QString icon;
    QString identifier;
    
    /** urls for protocol */
    QVariantMap m_urls;
};

class SourceItem : public QStandardItem
{
    public:
        SourceItem(const Source& s);
        virtual QVariant data(int role = Qt::UserRole + 1) const;
        
    private:
        Source m_s;
};

class KDEProjectsModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        
        enum Role { VcsLocationRole = Qt::UserRole+1, IdentifierRole };
        
        explicit KDEProjectsModel(QObject* parent = 0);
};

#endif // KDEPROJECTSMODEL_H
