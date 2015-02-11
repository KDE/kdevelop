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
#include "debug.h"
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <QTextDocument>

using namespace Cpp;
using namespace KDevelop;

class MissingDeclarationAction : public IAssistantAction
{
  Q_OBJECT
public:
  MissingDeclarationAction(const MissingDeclarationProblem::Ptr &problem)
  : m_problem(problem)
  {}
  // returns a full string representing the resulting declaration, without scope or access specifiers.
  virtual QString declarationString() const = 0;

protected:
  /**
   * @return the DUContext in which the declaration will be added.
   *
   * NOTE: The DUChain is locked when this is being called.
   */
  virtual DUContext* targetContext() const = 0;

  QString typeString(const AbstractType::Ptr& type) const
  {
    DUChainReadLocker lock;
    if(!type) {
      return "<no type>";
    } else if(DUContext* container = targetContext()) {
      return shortenedTypeString(type, container, 30);
    } else {
      return QString();
    }
  }

  AbstractType::Ptr type(const AbstractType::Ptr& base) const
  {
    DUChainReadLocker lock;
    AbstractType::Ptr ret = TypeUtils::removeConstants(base, m_problem->topContext());
    if(ret) {
      ret = TypeUtils::realTypeKeepAliases(ret);
      TypeUtils::removeConstModifier(ret);
    }
    return ret;
  }

  AbstractType::Ptr assignedType() const
  {
    return type(m_problem->type->assigned.type.abstractType());
  }

  MissingDeclarationProblem::Ptr m_problem;
};

class CreateLocalDeclarationAction : public MissingDeclarationAction
{
  Q_OBJECT
public:
    CreateLocalDeclarationAction(const MissingDeclarationProblem::Ptr& problem)
    : MissingDeclarationAction(problem)
    {
    }
    virtual void execute() override
    {
      DUChainReadLocker lock;
      if(targetContext()) {
        DocumentChangeSet changes;
        KTextEditor::Cursor start = m_problem->rangeInCurrentRevision().start();
        changes.addChange(DocumentChange(m_problem->url(), KTextEditor::Range(start, start),
                                         QString(), typeString(assignedType()) + " "));
        lock.unlock();

        changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
        changes.applyAllChanges();
        emit executed(this);
      }
    }
    virtual QString description() const override
    {
      return i18n("<b>local</b> variable");
    }
    virtual QString toolTip() const override
    {
      return i18n("Create local declaration %1", declarationString());
    }
    virtual QString declarationString() const override
    {
      return typeString(assignedType()) + " "
              + m_problem->type->identifier().toString();
    }
    virtual DUContext* targetContext() const override
    {
      return m_problem->type->searchStartContext.data();
    }
};

class CreateMemberDeclarationAction : public MissingDeclarationAction
{
  Q_OBJECT
public:
  CreateMemberDeclarationAction(const MissingDeclarationProblem::Ptr& problem,
                                Declaration::AccessPolicy access = Declaration::Public)
  : MissingDeclarationAction(problem)
  , m_access(access)
  {
  }
  virtual void execute() override
  {
    DUChainReadLocker lock;
    DUContext* searchFrom = m_problem->type->searchStartContext.data();
    DUContext* container = targetContext();

    if(searchFrom && container) {
      Cpp::SourceCodeInsertion ins(container->topContext());
      ins.setContext(container);
      ins.setAccess(m_access);

      if(m_problem->type->isFunction) {
        QList<Cpp::SourceCodeInsertion::SignatureItem> signature;
        int num = 1;
        QSet<QString> hadNames;
        foreach(const OverloadResolver::Parameter& arg, m_problem->type->arguments) {
          Cpp::SourceCodeInsertion::SignatureItem item;
          item.type = type(arg.type);
          item.name = QString("arg%1").arg(num);
          qCDebug(CPPDUCHAIN) << "have declaration: " << arg.declaration.data();

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
        ins.insertFunctionDeclaration(m_problem->type->identifier().identifier().identifier().last(), returnType(), signature);
      } else {
        ins.insertVariableDeclaration(m_problem->type->identifier().identifier().identifier().last(), returnType());
      }
      lock.unlock();

      ins.changes().setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
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
  virtual QString description() const override
  {
    return QString("<b>%1</b>").arg(accessString());
  }

  virtual QString containerString() const
  {
    DUChainReadLocker lock;
    DUContext* container = targetContext();
    if(container)
      return container->scopeIdentifier(true).toString();
    else
      return QString();
  }

  virtual QString declarationString() const override
  {
    DUChainReadLocker lock;
    if(targetContext())
      return returnString() + " " + m_problem->type->identifier().toString() + signatureString();
    else
      return QString();
  }

  virtual QString toolTip() const override
  {
    return i18nc("%1: access, %2: identifier/signature", "Declare %1 %2",
                  accessString(), declarationString());
  }
private:
  virtual DUContext* targetContext() const override
  {
    DUContext* container = m_problem->type->containerContext.data();
    if(!container) {
      Declaration* classDecl = localClassFromCodeContext(m_problem->type->searchStartContext.data());
      if(classDecl)
        container = classDecl->internalContext();
    }
    return container;
  }

  QString accessString() const
  {
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

  AbstractType::Ptr returnType() const
  {
    AbstractType::Ptr r = type(m_problem->type->convertedTo.type.abstractType());
    if(r)
      return r;

    r = assignedType();
    if(r) {
      if(m_problem->type->isFunction)
      {
        //A function that something is assigned to must return a reference
        ReferenceType::Ptr ref(new ReferenceType);
        ref->setBaseType(r);
        r = ref.cast<AbstractType>();
      }
      return r;
    }

    IntegralType* i = new IntegralType;
    i->setDataType(IntegralType::TypeVoid);
    return AbstractType::Ptr(i);
  }

  QString returnString() const
  {
    if (AbstractType::Ptr ret = returnType()) {
      return typeString(ret);
    } else {
      return QString();
    }
  }

  QString signatureString() const
  {
    if(!m_problem->type->isFunction) {
      return QString();
    }

    QString ret = "(";
    bool first = true;
    foreach(const OverloadResolver::Parameter& arg, m_problem->type->arguments) {
      if(!first)
        ret += ", ";
      else
        first = false;
      ret += typeString(arg.type);
    }
    ret += ")";
    return ret;
  }

  Declaration::AccessPolicy m_access;
};

MissingDeclarationAssistant::MissingDeclarationAssistant(const MissingDeclarationProblem::Ptr& p)
: problem(p)
, type(p->type)
{
  bool actionAdded = false;
  DUChainReadLocker lock;

  // support for namespaced types/variable names is broken atm, so just rather disable it
  // i.e. writing 'foo::bar()' will just add 'bar' to the container
  auto qualifiedIdentifier = p->type->identifier().identifier().identifier();
  if (qualifiedIdentifier.isEmpty() || qualifiedIdentifier.count() > 1)
    return;

  qCDebug(CPPDUCHAIN) << "creating assistant for" << type->toString() << "assigned:" << type->assigned.toString();

  DUContext* searchFrom = type->searchStartContext.data();
  if (!searchFrom) {
    return;
  }

  if(canCreateLocal(searchFrom)) {
    //Action to just copy in the type, i.e. create local declaration
    addAction(IAssistantAction::Ptr(new CreateLocalDeclarationAction(problem)));
  }

  Declaration* localClass = localClassFromCodeContext(searchFrom);
  Declaration* targetClass = 0;
  if (problem->type->containerContext.isValid()) {
    targetClass = localClassFromCodeContext(problem->type->containerContext.data());
  } else {
    targetClass = localClass;
  }

  if (canAddTo(targetClass, localClass)
      && (type->convertedTo.isValid() || type->assigned.isValid() || type->isFunction))
  {
    // public is always possible
    CreateMemberDeclarationAction* publicAction = new CreateMemberDeclarationAction(problem, Declaration::Public);

    if (actionAdded) {
      // place label between first action and the following actions
      const QString label = i18nc("assistant, declare member in class identified by %1",
                                  "member of <code>%1</code>:",
                                  publicAction->containerString()).toHtmlEscaped();
      addAction(IAssistantAction::Ptr(new AssistantLabelAction(label)));
    } else {
      // set the title manually
      m_title = i18nc("assistant, declare %1 as member of class identified by %2",
                      "Declare <code>'%1'</code> as member of <code>%2</code>",
                      publicAction->declarationString(),
                      publicAction->containerString()).toHtmlEscaped();
    }

    if(localClass == targetClass) {
      //Actions to create a declaration within the local class
      addAction(IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Private)));
      addAction(IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Protected)));
    } else if (localClass && targetClass
               && TypeUtils::isPublicBaseClass(localClass->type<CppClassType>(),
                                               targetClass->type<CppClassType>(),
                                               problem->topContext()))
    {
      //if we are in a subclass, we can provide the protected option
        addAction(IAssistantAction::Ptr(new CreateMemberDeclarationAction(problem, Declaration::Protected)));
    }
    // public is always possible
    addAction(IAssistantAction::Ptr(publicAction));
  }

  if(!actions().isEmpty() && m_title.isEmpty()) {
    MissingDeclarationAction* action = dynamic_cast<MissingDeclarationAction*>(actions().last().data());
    Q_ASSERT(action);
    m_title = i18n("Declare <code>'%1'</code> as", action->declarationString());
  }
}

bool MissingDeclarationAssistant::canCreateLocal(DUContext* searchFrom) const
{
  return !type->containerContext.data() && searchFrom->type() == DUContext::Other
    && type->assigned.type.isValid() && !type->assigned.type.type<DelayedType>()
    && !type->convertedTo.isValid();
}

bool MissingDeclarationAssistant::canAddTo(Declaration* toClass, Declaration* fromClass) const
{
  if (!toClass) {
    return false;
  }
  // same file, should be possible
  if (fromClass && fromClass->url() == toClass->url()) {
    return true;
  }
  // otherwise only if we currently edit the file, or if it's in an opened project
  const QUrl url = toClass->url().toUrl();
  return ICore::self()->projectController()->findProjectForUrl(url)
        || ICore::self()->documentController()->documentForUrl(url);
}

#include "missingdeclarationassistant.moc"
