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

#ifndef DOCUMENTATIONCONTROLLER_H
#define DOCUMENTATIONCONTROLLER_H

#include <interfaces/idocumentationcontroller.h>
#include <QObject>

namespace KDevelop
{
class Core;

class DocumentationController : public KDevelop::IDocumentationController, public QObject
{
    public:
        DocumentationController(Core* core);
        
        void initialize();
        
        virtual KSharedPtr< KDevelop::IDocumentation > documentationForDeclaration(KDevelop::Declaration* declaration);
};

}

#endif // DOCUMENTATIONCONTROLLER_H
