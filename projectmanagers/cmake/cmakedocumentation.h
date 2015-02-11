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
#include <QStringList>
#include <QVariantList>
#include <interfaces/iplugin.h>
#include "icmakedocumentation.h"

class QStringListModel;
namespace KDevelop { class Declaration; }
class CMakeManager;
class QUrl;

class CMakeDocumentation : public KDevelop::IPlugin, public ICMakeDocumentation
{
    Q_OBJECT
    Q_INTERFACES( ICMakeDocumentation )
    Q_INTERFACES( KDevelop::IDocumentationProvider )
    public:
        explicit CMakeDocumentation( QObject* parent = 0, const QVariantList& args = QVariantList() );

        virtual bool hasError() const override;
        virtual QString errorDescription() const override;

        KDevelop::IDocumentation::Ptr description(const QString& identifier, const QUrl &file) const override;
        KDevelop::IDocumentation::Ptr documentationForDeclaration(KDevelop::Declaration* declaration) const override;
        
        QStringList names(Type t) const override;
        
        QAbstractListModel* indexModel() const override;
        KDevelop::IDocumentation::Ptr documentationForIndex(const QModelIndex& idx) const override;
        
        virtual QIcon icon() const override;
        virtual QString name() const override;
        virtual KDevelop::IDocumentation::Ptr homePage() const override;
        
        QString descriptionForIdentifier(const QString& identifier, Type t) const;
    public slots:
        void delayedInitialization();
        
    Q_SIGNALS:
        void addHistory(const KDevelop::IDocumentation::Ptr& doc) const override;
    
    private:
        void initializeModel() const;
        void collectIds(const QString& param, Type type);
        
        QMap<QString, Type> m_typeForName;
        const QString mCMakeCmd;
        QStringListModel* m_index;
};

#endif // CMAKEDOCUMENTATION_H
