/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>

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

#ifndef KDEVPLATFORM_IDOCUMENTATIONPROVIDER_H
#define KDEVPLATFORM_IDOCUMENTATIONPROVIDER_H

#include <QtCore/QObject>
#include "interfacesexport.h"
#include "idocumentation.h"

class QIcon;
class QModelIndex;
class QAbstractListModel;
namespace KDevelop
{
class Declaration;

class KDEVPLATFORMINTERFACES_EXPORT IDocumentationProvider
{
public:
    virtual ~IDocumentationProvider();
    
    /** @returns an IDocument instance for the specified declaration or a null pointer if none could be found.*/
    virtual IDocumentation::Ptr documentationForDeclaration(KDevelop::Declaration* declaration) const = 0;

    /** @returns an instance of an interface to create an index for all the items provided by this class. */
    virtual QAbstractListModel* indexModel() const = 0;

    /** @returns the documentation information related to the index in the model. */
    virtual IDocumentation::Ptr documentationForIndex(const QModelIndex& idx) const = 0;

    /** @returns some icon associated to the provider. */
    virtual QIcon icon() const = 0;

    /** @returns a name to identify the provider to the user. */
    virtual QString name() const = 0;

    /** @returns a documentation item where we can show some home page information such a context index. */
    virtual IDocumentation::Ptr homePage() const = 0;

Q_SIGNALS:
    virtual void addHistory(const KDevelop::IDocumentation::Ptr& doc) const = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::IDocumentationProvider, "org.kdevelop.IDocumentationProvider")

#endif

