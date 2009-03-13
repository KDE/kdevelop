/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#include "missingdeclarationassistant.h"
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <klocalizedstring.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/types/constantintegraltype.h>
#include "typeutils.h"
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include "sourcemanipulation.h"
#include <language/duchain/duchainutils.h>
#include <../../kdevplatform/interfaces/idocumentationcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include "cppduchain.h"

using namespace Cpp;
using namespace KDevelop;

class CreateLocalDeclarationAction : public IAssistantAction {
    public:
        CreateLocalDeclarationAction(KSharedPtr< Cpp::MissingDeclarationProblem > _problem) : problem(_problem) {
        }
        virtual void execute() {
          DUChainReadLocker lock(DUChain::lock());
//           if(DUContext* searchFrom = problem->type->searchStartContext.data()) {
            KDevelop::DocumentChangeSet changes;
            changes.addChange(KDevelop::DocumentChange(problem->url(), SimpleRange(problem->range().start, problem->range().start), QString(),  type()->toString() + " "));
            lock.unlock();
            changes.applyAllChanges(KDevelop::DocumentChangeSet::WarnOnFailedChange);
//           }
        }
        virtual QString description() const {
          return i18n("Create local declaration %1 %2", type()->toString(), problem->type->identifier().toString());
        }
    private:
        AbstractType::Ptr type() const {
          return TypeUtils::realType(TypeUtils::removeConstants(problem->type->assigned.type.abstractType()));
        }
        KSharedPtr< Cpp::MissingDeclarationProblem > problem;
        QString m_description;
};

class CreateMemberDeclarationAction : public IAssistantAction {
    public:
        CreateMemberDeclarationAction(KSharedPtr< Cpp::MissingDeclarationProblem > _problem, Declaration::AccessPolicy access = Declaration::Public) : problem(_problem), m_access(access) {
        }
        virtual void execute() {
          DUChainReadLocker lock(DUChain::lock());
          DUContext* searchFrom = problem->type->searchStartContext.data();
          DUContext* container = useContainer();
          
          if(searchFrom && container) {
            Cpp::SourceCodeInsertion ins(container->topContext());
            ins.setContext(container);
            ins.setAccess(m_access);
            
            if(problem->type->isFunction) {
              QList<Cpp::SourceCodeInsertion::SignatureItem> signature;
              int num = 1;
              foreach(OverloadResolver::Parameter arg, problem->type->arguments) {
                Cpp::SourceCodeInsertion::SignatureItem item;
                item.type = arg.type;
                item.name = QString("arg%1").arg(num);
                signature << item;
                ++num;
              }
              ins.insertFunctionDeclaration(problem->type->identifier().last(), returnType(), signature);
            }else{
              ins.insertVariableDeclaration(problem->type->identifier().last(), returnType());
            }
            lock.unlock();
            ins.changes().applyAllChanges(KDevelop::DocumentChangeSet::WarnOnFailedChange);
          }
        }
        virtual QString description() const {
          DUChainReadLocker lock(DUChain::lock());
          DUContext* container = useContainer();
          if(container)
            return i18n("Declarate %1 %2 %3",  accessString(), returnString(), container->scopeIdentifier(true).toString() + "::" + problem->type->identifier().toString() + signatureString());
          else
            return QString();
        }
    private:
        QString accessString() const {
          switch(m_access) {
            case Declaration::Protected:
              return "protected";
            case Declaration::Private:
              return "private";
          }
          return QString();
        }
      
        DUContext* useContainer() const {
          DUContext* container = problem->type->containerContext.data();
          if(!container) {
            Declaration* classDecl = Cpp::localClassFromCodeContext(problem->type->searchStartContext.data());
            if(classDecl)
              container = classDecl->internalContext();
          }
          return container;
        }
      
        QString returnString() const {
          if(returnType()){
            return returnType()->toString();
          }else {
            return QString();
          }
        }
        
        QString signatureString() const {
          if(problem->type->isFunction) {
            QString ret = "(";
            bool first = true;
            foreach(OverloadResolver::Parameter arg, problem->type->arguments) {
              if(!first)
                ret += ", ";
              else
                first = false;
              ret += typeString(arg.type);
            }
            ret += ")";
            return ret;
          }
          return QString();
        }
        
        AbstractType::Ptr returnType() const {
          AbstractType::Ptr r = type(problem->type->convertedTo.type.abstractType());
          if(r)
            return r;
          
          r = type(problem->type->assigned.type.abstractType());
          if(r) {
            if(problem->type->isFunction)
            {
              //A function that something is assigned to must return a reference
              ReferenceType::Ptr ref(new ReferenceType);
              ref->setBaseType(r);
              r = ref.cast<AbstractType>();
            }
            return r;
          }
          
          KDevelop::IntegralType* i = new KDevelop::IntegralType;
          i->setDataType(KDevelop::IntegralType::TypeVoid);
          return AbstractType::Ptr(i);
        }
        
        QString typeString(AbstractType::Ptr base) const {
          AbstractType::Ptr t = type(base);
          if(t)
            return t->toString();
          else
            return "<no type>";
        }
        
        AbstractType::Ptr type(AbstractType::Ptr base) const {
          return TypeUtils::realType(TypeUtils::removeConstants(base));
        }
      
        KSharedPtr< Cpp::MissingDeclarationProblem > problem;
        QString m_description;
        Declaration::AccessPolicy m_access;
};

MissingDeclarationAssistant::MissingDeclarationAssistant(KSharedPtr< Cpp::MissingDeclarationProblem > p) : problem(p), type(p->type) {
  DUChainReadLocker lock(DUChain::lock());
  if(p->type->identifier().isEmpty())
    return;
  kDebug() << "creating assistant for" << type->toString() << "assigned:" << type->assigned.toString();;
  
  if(DUContext* searchFrom = type->searchStartContext.data()) {
    if(!type->containerContext.data() && searchFrom->type() == DUContext::Other && (type->assigned.type.abstractType() || type->isFunction))
    {
      //Action to just copy in the type
      if(!type->assigned.type.type<KDevelop::DelayedType>() && !type->isFunction && !type->convertedTo.isValid())
        addAction(KDevelop::IAssistantAction::Ptr(new CreateLocalDeclarationAction(problem)));
      Declaration* localClass =Cpp::localClassFromCodeContext(searchFrom);
      
      //Action to create a declaration within the local class
      if(localClass && localClass->internalContext()) {
        addAction(KDevelop::IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Public)));
        addAction(KDevelop::IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Private)));
      }
    }
    if(type->containerContext.data() && (type->convertedTo.isValid() || type->assigned.isValid() || type->isFunction)) {
      KUrl url = type->containerContext.data()->url().toUrl();
      if(KDevelop::ICore::self()->projectController()->findProjectForUrl(url) || KDevelop::ICore::self()->documentController()->documentForUrl(url))
        addAction(KDevelop::IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem)));
    }
  }
}
