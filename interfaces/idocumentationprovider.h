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

#ifndef IDOCUMENTATIONPROVIDER_H
#define IDOCUMENTATIONPROVIDER_H

#include <KSharedPtr>

#include "iextension.h"
#include "interfacesexport.h"

namespace KDevelop
{
class Declaration;
class IDocumentation;

class KDEVPLATFORMINTERFACES_EXPORT IDocumentationProvider
{
    public:
        virtual ~IDocumentationProvider();
        
        /** @returns an IDocument instance for the specified declaration or a null pointer if none could be found.*/
        virtual KSharedPtr<IDocumentation> documentationForDeclaration(KDevelop::Declaration* declaration) = 0;
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, IDocumentationProvider, "org.kdevelop.IDocumentationProvider")
Q_DECLARE_INTERFACE( KDevelop::IDocumentationProvider, "org.kdevelop.IDocumentationProvider")

#endif

