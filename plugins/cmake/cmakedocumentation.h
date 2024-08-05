/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
        explicit CMakeDocumentation(QObject* parent, const KPluginMetaData& metaData,
                                    const QVariantList& args = QVariantList());

        KDevelop::IDocumentation::Ptr description(const QString& identifier, const QUrl &file) const override;
        KDevelop::IDocumentation::Ptr documentationForDeclaration(KDevelop::Declaration* declaration) const override;
        KDevelop::IDocumentation::Ptr documentation(const QUrl& url) const override;
        
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
