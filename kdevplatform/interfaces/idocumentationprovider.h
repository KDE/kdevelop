/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IDOCUMENTATIONPROVIDER_H
#define KDEVPLATFORM_IDOCUMENTATIONPROVIDER_H

#include "interfacesexport.h"
#include <QObject>
#include <QExplicitlySharedDataPointer>

class QIcon;
class QModelIndex;
class QUrl;
class QAbstractItemModel;
namespace KDevelop
{
class Declaration;
class IDocumentation;
using IDocumentationPtr = QExplicitlySharedDataPointer<IDocumentation>;

class KDEVPLATFORMINTERFACES_EXPORT IDocumentationProvider
{
public:
    virtual ~IDocumentationProvider();
    
    /** @returns an IDocument instance for the specified declaration or a null pointer if none could be found.*/
    virtual IDocumentationPtr documentationForDeclaration(KDevelop::Declaration* declaration) const = 0;

    /** @returns an IDocumentation instance for the specified URL if the provider has a concept of
     *  mapping URLS to Documentations and has a Documentation related to the queried URL. Otherwise
     *  returns a null pointer. */
    virtual IDocumentationPtr documentation(const QUrl& url) const = 0;

    /** @returns an instance of an interface to create an index for all the items provided by this class.
     *  Should have all items at the same level */
    virtual QAbstractItemModel* indexModel() const = 0;

    /** @returns the documentation information related to the index in the model. */
    virtual IDocumentationPtr documentationForIndex(const QModelIndex& idx) const = 0;

    /** @returns some icon associated to the provider. */
    virtual QIcon icon() const = 0;

    /** @returns a name to identify the provider to the user. */
    virtual QString name() const = 0;

    /** @returns a documentation item where we can show some home page information such a context index. */
    virtual IDocumentationPtr homePage() const = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::IDocumentationProvider, "org.kdevelop.IDocumentationProvider")

#endif

