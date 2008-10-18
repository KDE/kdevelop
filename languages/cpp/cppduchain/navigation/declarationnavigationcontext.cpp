/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "declarationnavigationcontext.h"

#include <QtGui/QTextDocument>

#include <klocale.h>

#include <language/duchain/functiondefinition.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/duchainutils.h>

#include "../typeutils.h"
#include "../templatedeclaration.h"
#include "../classdeclaration.h"

namespace Cpp {

DeclarationNavigationContext::DeclarationNavigationContext( DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, AbstractNavigationContext* previousContext)
  : AbstractDeclarationNavigationContext( decl, topContext, previousContext )
{
}


NavigationContextPointer DeclarationNavigationContext::registerChild(DeclarationPointer declaration)
{
  return AbstractNavigationContext::registerChild(new DeclarationNavigationContext(declaration, m_topContext, this));
}

void DeclarationNavigationContext::htmlClass()
{
  AbstractDeclarationNavigationContext::htmlClass();

  StructureType::Ptr klass = m_declaration->abstractType().cast<StructureType>();
  Q_ASSERT(klass);

  Cpp::ClassDeclaration* classDecl = dynamic_cast<Cpp::ClassDeclaration*>(klass->declaration(m_topContext.data()));
  if(classDecl) {
    FOREACH_FUNCTION( const Cpp::BaseClassInstance& base, classDecl->baseClasses ) {
      m_currentText += ", " + stringFromAccess(base.access) + " " + (base.virtualInheritance ? QString("virtual") : QString()) + " ";
      eventuallyMakeTypeLinks(base.baseClass.type());
    }
    m_currentText += " ";
  }
}

void DeclarationNavigationContext::htmlIdentifiedType(AbstractType::Ptr type, const IdentifiedType* idType)
{
  AbstractDeclarationNavigationContext::htmlIdentifiedType(type, idType);

  if( const TemplateDeclaration* templ = dynamic_cast<const TemplateDeclaration*>(idType->declaration(m_topContext.data())) ) {
    if( templ->instantiatedFrom() ) {
      m_currentText += Qt::escape("  <");

      const Cpp::InstantiationInformation& params = templ->instantiatedWith().information();
      bool first = true;
      FOREACH_FUNCTION( const IndexedType& type, params.templateParameters ) {
        if( first )
          first = false;
        else
          m_currentText += ", ";

        if( type ) {
          AbstractType::Ptr t = type.type();
          eventuallyMakeTypeLinks(t);
        }else{
            m_currentText += "missing type";
        }
      }

      m_currentText += Qt::escape(" >");
    }
  }
}

}
