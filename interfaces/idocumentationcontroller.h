/*
   Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
   
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

#ifndef IDOCUMENTATIONCONTROLLER_H
#define IDOCUMENTATIONCONTROLLER_H

#include <interfaces/idocumentation.h>

namespace KDevelop {
class IDocumentationProvider;
class Declaration;

/**
 * Allows to access the documentation.
 *
 * @author Aleix Pol <aleixpol@kde.org>
 */
class KDEVPLATFORMINTERFACES_EXPORT IDocumentationController
{
    public:
        IDocumentationController();
        virtual ~IDocumentationController();
        
        /** Return the documentation provider plugin instances. */
        virtual QList<IDocumentationProvider*> documentationProviders() const=0;
        
        /** Return the corresponding documentation instance for a determinate declaration. */
        virtual KSharedPtr<IDocumentation> documentationForDeclaration(Declaration* declaration)=0;
        
        /** Show the documentation specified by @p doc. */
        virtual void showDocumentation(KSharedPtr< KDevelop::IDocumentation > doc)=0;
};

}

#endif
