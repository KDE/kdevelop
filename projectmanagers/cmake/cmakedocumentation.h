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
class KUrl;

class CMakeDocumentation : public KDevelop::IPlugin, public ICMakeDocumentation
{
    Q_OBJECT
    Q_INTERFACES( ICMakeDocumentation )
    Q_INTERFACES( KDevelop::IDocumentationProvider )
    public:
        explicit CMakeDocumentation( QObject* parent = 0, const QVariantList& args = QVariantList() );

        virtual bool hasError() const;
        virtual QString errorDescription() const;

        KSharedPtr<KDevelop::IDocumentation> description(const QString& identifier, const KUrl& file) const;
        KSharedPtr<KDevelop::IDocumentation> documentationForDeclaration(KDevelop::Declaration* declaration) const;
        
        QStringList names(Type t) const;
        
        QAbstractListModel* indexModel() const;
        KSharedPtr<KDevelop::IDocumentation> documentationForIndex(const QModelIndex& idx) const;
        
        virtual QIcon icon() const;
        virtual QString name() const;
        virtual KSharedPtr<KDevelop::IDocumentation> homePage() const;
        
        QString descriptionForIdentifier(const QString& identifier, Type t) const;
    public slots:
        void delayedInitialization();
        
    Q_SIGNALS:
        void addHistory(const KSharedPtr< KDevelop::IDocumentation >& doc) const;
    
    private:
        void initializeModel() const;
        void collectIds(const QString& param, Type type);
        
        QMap<QString, Type> m_typeForName;
        const QString mCMakeCmd;
        QStringListModel* m_index;
};

#endif // CMAKEDOCUMENTATION_H
