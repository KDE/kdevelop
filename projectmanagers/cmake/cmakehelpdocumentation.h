/* KDevelop CMake Support
 *
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEHELPDOCUMENTATION_H
#define CMAKEHELPDOCUMENTATION_H

#include <interfaces/idocumentation.h>
#include <QtCore/QAbstractItemModel>

class CMakeContentsModel : public QAbstractItemModel
{
    Q_OBJECT
    public:       
        CMakeContentsModel(QObject* parent) ;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        
        int columnCount(const QModelIndex&) const override { return 1; }
        QModelIndex parent(const QModelIndex& child) const override;
        
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role) const override;
    public slots:
        void showItem(const QModelIndex& idx);
};

class CMakeHomeDocumentation : public KDevelop::IDocumentation
{
    public:
        virtual KDevelop::IDocumentationProvider* provider() const override;
        virtual QString name() const override;
        virtual QString description() const override { return name(); }

        virtual QWidget* documentationWidget ( KDevelop::DocumentationFindWidget* findWidget, QWidget* parent = 0 ) override;
};

#endif
