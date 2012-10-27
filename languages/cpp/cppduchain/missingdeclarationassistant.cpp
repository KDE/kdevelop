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
#include <interfaces/idocumentcontroller.h>
#include "cppduchain.h"
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <QTextDocument>

using namespace Cpp;
using namespace KDevelop;

class MissingDeclarationAction : public IAssistantAction {
public:
  // returns a full string representing the resulting declaration, without scope or access specifiers.
  virtual QString getDeclarationString() const = 0;
  // returns a full string representing the container of the resulting declaration (only if it is a class/struct)
  virtual QString getContainerString() const {
    return QString();
  }
};

class CreateLocalDeclarationAction : public MissingDeclarationAction {
    public:
        CreateLocalDeclarationAction(KSharedPtr< Cpp::MissingDeclarationProblem > _problem) : problem(_problem) {
        }
        virtual void execute() {
          DUChainReadLocker lock(DUChain::lock());
          if(/*DUContext* searchFrom = */problem->type->searchStartContext.data()) {
            KDevelop::DocumentChangeSet changes;
            changes.addChange(KDevelop::DocumentChange(problem->url(), SimpleRange(problem->rangeInCurrentRevision().start, problem->rangeInCurrentRevision().start), QString(),  typeString() + " "));
            lock.unlock();
            
            changes.setReplacementPolicy(KDevelop::DocumentChangeSet::WarnOnFailedChange);
            changes.applyAllChanges();
            emit executed(this);
          }
        }
        virtual QString description() const {
          return i18n("<b>local</b> variable");
        }
        virtual QString toolTip() const {
          return i18n("Create local declaration %1", getDeclarationString());
        }
        
        virtual QString getDeclarationString() const {
          return typeString() + " " + problem->type->identifier().toString();
        }
        
        QString typeString(int maxSize = 10000) const {
          DUChainReadLocker lock(DUChain::lock());
          if(DUContext* searchFrom = problem->type->searchStartContext.data())
            return Cpp::shortenedTypeString(type(), searchFrom, maxSize);
          else
            return QString();
        }
        
    private:
      
        AbstractType::Ptr type() const {
          AbstractType::Ptr ret = TypeUtils::realTypeKeepAliases(TypeUtils::removeConstants(problem->type->assigned.type.abstractType(), problem->topContext()))->indexed().abstractType();
          if(ret)
            ret->setModifiers(ret->modifiers() & (~AbstractType::ConstModifier)); //Remove "const" modifier
          return ret;
        }
        KSharedPtr< Cpp::MissingDeclarationProblem > problem;
        QString m_description;
};

class CreateMemberDeclarationAction : public MissingDeclarationAction {
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
              QSet<QString> hadNames;
              foreach(const OverloadResolver::Parameter& arg, problem->type->arguments) {
                Cpp::SourceCodeInsertion::SignatureItem item;
                item.type = type(arg.type);
                item.name = QString("arg%1").arg(num);
                kDebug() << "have declaration: " << arg.declaration.data();
                
                if(arg.declaration.data())
                {
                  // Find a unique name
                  QString baseName = arg.declaration.data()->identifier().identifier().str();
                  for(int a = 1; a < 1000; ++a)
                  {
                    if(!hadNames.contains(baseName))
                    {
                      item.name = baseName;
                      break;
                    }
                    QString name = QString(baseName + "%1").arg(a);
                    if(!hadNames.contains(name))
                    {
                      item.name = name;
                      break;
                    }
                  }
                }
                
                signature << item;
                ++num;
              }
              ins.insertFunctionDeclaration(problem->type->identifier().identifier().identifier().last(), returnType(), signature);
            }else{
              ins.insertVariableDeclaration(problem->type->identifier().identifier().identifier().last(), returnType());
            }
            lock.unlock();
            
            ins.changes().setReplacementPolicy(KDevelop::DocumentChangeSet::WarnOnFailedChange);
            ins.changes().applyAllChanges();
            const IndexedString localUrl = searchFrom->url();
            const IndexedString changeUrl = container->url();
            if(changeUrl != localUrl) {
              ICore::self()->languageController()->backgroundParser()->addDocument(changeUrl);
              ICore::self()->languageController()->backgroundParser()->addDocument(localUrl);
            }
            emit executed(this);
          }
        }
        virtual QString description() const {
          if(problem->type->isFunction)
            return i18n("<b>%1</b> function in <i>%2</i>", accessString(), Qt::escape(getContainerString()));
          else
            return i18n("<b>%1</b> variable in <i>%2</i>", accessString(), Qt::escape(getContainerString()));
        }
        
        virtual QString getContainerString() const {
          DUChainReadLocker lock(DUChain::lock());
          DUContext* container = useContainer();
          if(container)
            return container->scopeIdentifier(true).toString();
          else
            return QString();
        }
        
        virtual QString getDeclarationString() const {
          DUChainReadLocker lock(DUChain::lock());
          DUContext* container = useContainer();
          if(container)
            return QString("%2 %3").arg(returnString(), problem->type->identifier().toString() + signatureString());
          else
            return QString();
        }
        
        virtual QString toolTip() const {
          return QString("Declare %2 %3").arg(returnString(), getContainerString() + "::" + problem->type->identifier().toString() + signatureString());
        }
    private:
        QString accessString() const {
          switch(m_access) {
            case Declaration::Protected:
              return "protected";
            case Declaration::Private:
              return "private";
            case Declaration::Public:
              return "public";
            default:
              return QString();
          }      
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
      
        QString typeString(AbstractType::Ptr type) const {
          DUChainReadLocker lock(DUChain::lock());
          if(!type)
            return "<no type>";
          if(DUContext* container = useContainer())
            return Cpp::shortenedTypeString(type, container, 30);
          else
            return QString();
        }
      
        QString returnString() const {
          if(returnType()){
            return typeString(returnType());
          }else {
            return QString();
          }
        }
        
        QString signatureString() const {
          if(problem->type->isFunction) {
            QString ret = "(";
            bool first = true;
            foreach(const OverloadResolver::Parameter& arg, problem->type->arguments) {
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
        
        AbstractType::Ptr type(AbstractType::Ptr base) const {
          DUChainReadLocker lock;
          AbstractType::Ptr ret = TypeUtils::realTypeKeepAliases(TypeUtils::removeConstants(base, problem->topContext()))->indexed().abstractType();
          if(ret)
            ret->setModifiers(ret->modifiers() & (~AbstractType::ConstModifier)); //Remove "const" modifier
          return ret;
        }
      
        KSharedPtr< Cpp::MissingDeclarationProblem > problem;
        QString m_description;
        Declaration::AccessPolicy m_access;
};

MissingDeclarationAssistant::MissingDeclarationAssistant(KSharedPtr< Cpp::MissingDeclarationProblem > p) : problem(p), type(p->type) {
  DUChainReadLocker lock(DUChain::lock());
  if(p->type->identifier().identifier().identifier().isEmpty())
    return;
  kDebug() << "creating assistant for" << type->toString() << "assigned:" << type->assigned.toString();
  
  if(DUContext* searchFrom = type->searchStartContext.data()) {
    if(!type->containerContext.data() && searchFrom->type() == DUContext::Other && (type->assigned.type.abstractType() || type->isFunction))
    {
      //Action to just copy in the type
      if(!type->assigned.type.type<KDevelop::DelayedType>() && !type->isFunction && !type->convertedTo.isValid()) {
        addAction(KDevelop::IAssistantAction::Ptr(new CreateLocalDeclarationAction(problem)));
      }
      
      Declaration* localClass = Cpp::localClassFromCodeContext(searchFrom);
      
      //Action to create a declaration within the local class
      if(localClass && localClass->internalContext()) {
        addAction(KDevelop::IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Private)));
        addAction(KDevelop::IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Protected)));
        addAction(KDevelop::IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Public)));
      }
      return;
    }
    if(type->containerContext.data() && (type->convertedTo.isValid() || type->assigned.isValid() || type->isFunction)) {
      KUrl url = type->containerContext.data()->url().toUrl();
      if(KDevelop::ICore::self()->projectController()->findProjectForUrl(url) || KDevelop::ICore::self()->documentController()->documentForUrl(url)) {
        DUContext* container = problem->type->containerContext.data();
        Declaration* searchFromClass = Cpp::localClassFromCodeContext(searchFrom);

        if (searchFromClass){
          if (searchFromClass->internalContext() == container){
            //we are in the same class, so we can provide private, protected and public
            addAction(KDevelop::IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Private)));
            addAction(KDevelop::IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Protected)));
          } else if (Declaration* containerClass = Cpp::localClassFromCodeContext(problem->type->containerContext.data())) {
            //if we are in a subclass, we can provide the protected option
            CppClassType::Ptr searchFromClassType = CppClassType::Ptr::dynamicCast(searchFromClass->abstractType());
            CppClassType::Ptr containerClassType = CppClassType::Ptr::dynamicCast(containerClass->abstractType());

            int levelCount;
            if (TypeUtils::isPublicBaseClass(searchFromClassType, containerClassType, container->topContext(), &levelCount )){
              addAction(KDevelop::IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Protected)));
            }
          }
        }

        //public is always possible
        addAction(KDevelop::IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Public)));
      }
    }
  }
  
  if(!actions().isEmpty())
  {
    MissingDeclarationAction* action = dynamic_cast<MissingDeclarationAction*>(actions().last().data());
    Q_ASSERT(action);
    m_title = i18n("Declare <big><tt>'%1'</tt></big> as", action->getDeclarationString());
  }
}
