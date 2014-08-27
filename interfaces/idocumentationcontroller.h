/*
   Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
   Copyright 2010 Benjamin Port <port.benjamin@gmail.com>
   
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

#ifndef KDEVPLATFORM_IDOCUMENTATIONCONTROLLER_H
#define KDEVPLATFORM_IDOCUMENTATIONCONTROLLER_H

#include <interfaces/idocumentation.h>
#include <QtCore/QObject>

namespace KDevelop {
class IDocumentationProvider;
class Declaration;

/**
 * Allows to access the documentation.
 *
 * @author Aleix Pol <aleixpol@kde.org>
 */
class KDEVPLATFORMINTERFACES_EXPORT IDocumentationController: public QObject
{
    Q_OBJECT
    public:
        IDocumentationController();
        virtual ~IDocumentationController();

        /** Return the documentation provider plugin instances. */
        virtual QList<IDocumentationProvider*> documentationProviders() const=0;
        
        /** Return the corresponding documentation instance for a determinate declaration. */
        virtual QExplicitlySharedDataPointer<IDocumentation> documentationForDeclaration(Declaration* declaration)=0;
        
        /** Show the documentation specified by @p doc. */
        virtual void showDocumentation(QExplicitlySharedDataPointer< KDevelop::IDocumentation > doc)=0;

    public Q_SLOTS:
        /** Emit signal when the documentation providers list changed. */
        virtual void changedDocumentationProviders() = 0;

    Q_SIGNALS:
        /** Emitted when providers list changed */
        void providersChanged();
};

}

#endif
