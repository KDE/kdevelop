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

#ifndef CMAKEDOCUMENTATION_H
#define CMAKEDOCUMENTATION_H

#include <QString>
#include <QMap>
#include <QVariantList>
#include <interfaces/iplugin.h>
#include "icmakedocumentation.h"

namespace KDevelop { class Declaration; }
class CMakeManager;
class QUrl;
class CMakeCommandsContents;
class KDescendantsProxyModel;

class CMakeDocumentation : public KDevelop::IPlugin, public ICMakeDocumentation
{
    Q_OBJECT
    Q_INTERFACES( ICMakeDocumentation )
    Q_INTERFACES( KDevelop::IDocumentationProvider )
    public:
        explicit CMakeDocumentation( QObject* parent = nullptr, const QVariantList& args = QVariantList() );

        KDevelop::IDocumentation::Ptr description(const QString& identifier, const QUrl &file) const override;
        KDevelop::IDocumentation::Ptr documentationForDeclaration(KDevelop::Declaration* declaration) const override;
        
        QVector<QString> names(Type t) const override;
        
        CMakeCommandsContents* model() const { return m_index; }
        QAbstractItemModel* indexModel() const override;
        KDevelop::IDocumentation::Ptr documentationForIndex(const QModelIndex& idx) const override;
        
        QIcon icon() const override;
        QString name() const override;
        KDevelop::IDocumentation::Ptr homePage() const override;
        
        QString descriptionForIdentifier(const QString& identifier, Type t) const;
    
    private:
        CMakeCommandsContents* m_index;
        KDescendantsProxyModel* m_flatIndex;
};

#endif // CMAKEDOCUMENTATION_H
