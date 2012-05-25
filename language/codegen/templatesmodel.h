/*  This file is part of KDevelop
    Copyright 2007 Alexander Dymo <adymo@kdevelop.org>
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

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

#ifndef KDEVELOP_TEMPLATESMODEL_H
#define KDEVELOP_TEMPLATESMODEL_H

#include <QStandardItemModel>

#include "../languageexport.h"

class KComponentData;
namespace KDevelop
{
class TemplatesModelPrivate;

class KDEVPLATFORMLANGUAGE_EXPORT TemplatesModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit TemplatesModel(const KComponentData& componentData, QObject* parent = 0);
    virtual ~TemplatesModel();
    
    virtual void refresh();
    QModelIndexList templateIndexes(const QString& fileName);
    
    void setDescriptionResourceType(const QByteArray& type);
    QByteArray descriptionResourceType() const;
    
    void setTemplateResourceType(const QByteArray& type);
    QByteArray templateResourceType() const;
        
protected:
    void extractTemplateDescriptions();
    bool templateExists(const QString&);
    QStandardItem *createItem(const QString &name, const QString &category);
    
private:
    TemplatesModelPrivate* const d;
};

}

#endif // KDEVELOP_TEMPLATESMODEL_H
