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

#include <QTextDocument>

#include <klocale.h>

#include <language/duchain/functiondefinition.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/forwarddeclaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>

#include "../typeutils.h"
#include "../templatedeclaration.h"
#include <language/duchain/classdeclaration.h>
#include "../cppduchain.h"

namespace Cpp {

DeclarationNavigationContext::DeclarationNavigationContext( DeclarationPointer decl, KDevelop::TopDUContextPointer topContext, AbstractNavigationContext* previousContext)
  : AbstractDeclarationNavigationContext( decl, topContext, previousContext )
{
}

QualifiedIdentifier DeclarationNavigationContext::prettyQualifiedIdentifier(DeclarationPointer decl) const
{
  QualifiedIdentifier ret;
  if(m_topContext.data() && decl) {
      if(decl->kind() == Declaration::Type) {
        AbstractType::Ptr type = decl->abstractType();
        DelayedType::Ptr stripped = stripType(type, m_topContext.data()).cast<DelayedType>();
        if(stripped)
          return stripped->identifier().identifier().identifier();
        else
          return decl->qualifiedIdentifier();
      }else{
        if(decl->context()->owner()) {
          return prettyQualifiedIdentifier(DeclarationPointer(decl->context()->owner())) + decl->identifier();
        }else{
          return decl->qualifiedIdentifier();
        }
      }
  }
  return ret;
}

KDevelop::AbstractType::Ptr DeclarationNavigationContext::typeToShow(KDevelop::AbstractType::Ptr type) {
  return shortenTypeForViewing(type);
}

void DeclarationNavigationContext::setPreviousContext(KDevelop::AbstractNavigationContext* previous) {
  AbstractDeclarationNavigationContext::setPreviousContext(previous);
  DUChainReadLocker lock(KDevelop::DUChain::lock());
  if(previous->topContext()) {
    if(m_declaration.data() && dynamic_cast<TemplateDeclaration*>(m_declaration.data()))
      //If this is a template, take the top-context from the previous context, because we cannot have correct visibility from here
      setTopContext(previous->topContext());
  }
}

void DeclarationNavigationContext::htmlIdentifiedType(AbstractType::Ptr type, const IdentifiedType* idType)
{
  AbstractDeclarationNavigationContext::htmlIdentifiedType(type, idType);

  if( const TemplateDeclaration* templ = dynamic_cast<const TemplateDeclaration*>(idType->declaration(m_topContext.data())) ) {
    if( templ->instantiatedFrom() ) {

      const Cpp::InstantiationInformation& params = templ->instantiatedWith().information();
      
      if(params.templateParametersSize() == 0)
        return;
      
      modifyHtml() += Qt::escape("  <");
      bool first = true;
      
      uint showParameters = params.templateParametersSize();
      
      if(m_topContext.data()) {
        //Strip away default-parameters, as they can be overwhelmingly ugly, especially in STL
        DelayedType::Ptr stripped = stripType(type, m_topContext.data()).cast<DelayedType>();
        if(stripped) {
            QualifiedIdentifier id = stripped->identifier().identifier().identifier();
            if(!id.isEmpty() && id.last().templateIdentifiersCount() < showParameters)
              showParameters = id.last().templateIdentifiersCount();
        }
      }
      
      
      for( uint p = 0; p < showParameters; ++p ) {
      const IndexedType& type(params.templateParameters()[p]);
      
        if( first )
          first = false;
        else
          modifyHtml() += ", ";

        if( type ) {
          AbstractType::Ptr t = type.abstractType();
          eventuallyMakeTypeLinks(t);
        }else{
            modifyHtml() += "missing type";
        }
      }

      modifyHtml() += Qt::escape(" >");
    }
  }
}

}
