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

#ifndef KDEVPLATFORM_DOCUMENTATIONCONTROLLER_H
#define KDEVPLATFORM_DOCUMENTATIONCONTROLLER_H

#include <interfaces/idocumentationcontroller.h>

class DocumentationViewFactory;

class QAction;

namespace KDevelop
{

class IDocumentationProvider;
class Core;
class Context;
class ContextMenuExtension;

class DocumentationController : public KDevelop::IDocumentationController
{
        Q_OBJECT
    public:
        DocumentationController(Core* core);

        void initialize();

        virtual QList<IDocumentationProvider*> documentationProviders() const;
        virtual QExplicitlySharedDataPointer< KDevelop::IDocumentation > documentationForDeclaration(KDevelop::Declaration* declaration);
        virtual void showDocumentation(QExplicitlySharedDataPointer< KDevelop::IDocumentation > doc);
        ContextMenuExtension contextMenuExtension( Context* context );

    public slots:
        virtual void changedDocumentationProviders();
    private slots:
        void doShowDocumentation();
    private:
        DocumentationViewFactory* m_factory;

        QAction* m_showDocumentation;
};

}

#endif // KDEVPLATFORM_DOCUMENTATIONCONTROLLER_H
