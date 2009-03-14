/*
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

#include "sourcemanipulation.h"
#include <language/codegen/coderepresentation.h>
#include "qtfunctiondeclaration.h"
#include "declarationbuilder.h"
#include "environmentmanager.h"
#include "templateparameterdeclaration.h"

using namespace KDevelop;

///Makes sure the line is not in a comment, moving it behind if needed. Just does very simple matching, should be ok for header copyright-notices and such.
int KDevelop::SourceCodeInsertion::firstValidCodeLineBefore(int lineNumber) const {
  
  if(lineNumber < 100) {
    int checkLines = m_codeRepresentation->lines() < 100 ? m_codeRepresentation->lines() : 100;
    
    bool inComment = false; //This is a bit stupid
    bool unsure = false;
    int unsureStart = -1;
    
    for(int a = 0; a < checkLines; ++a) {
      QString line = m_codeRepresentation->line(a).trimmed();
///@todo Use the "unsure" logic to jump over #ifdefs      
//       if(!inComment && !unsure && line.isEmpty()) {
//         unsure = true;
//         unsureStart = a;
//       }
//       
//       if(!inComment && line.startsWith("#")) {
//         unsure = true;
//         unsureStart = a+1;
//       }
      
      if(!inComment && !line.startsWith("//") && a >= lineNumber && !unsure)
        return a;
      
      if(line.indexOf("/*") != -1) {
        inComment = true;
        unsure = false;
      }

      if(line.indexOf("*/") != -1) {
        inComment = false;
        unsure = false;
      }
    }
//     if(line > unsureStart)
//       return unsureStart;
  }
  
  return lineNumber;
}

KDevelop::DocumentChangeSet& KDevelop::SourceCodeInsertion::changes() {
  return m_changeSet;
}

void KDevelop::SourceCodeInsertion::setInsertBefore(KDevelop::SimpleCursor position) {
  m_insertBefore = position;
}

void KDevelop::SourceCodeInsertion::setContext(KDevelop::DUContext* context) {
  m_context = context;
}

void KDevelop::SourceCodeInsertion::setSubScope(KDevelop::QualifiedIdentifier scope) {
  m_scope = scope;
  
  DUContext* context = m_topContext;
  if(m_context)
    context = m_context;
  
  if(!context)
    return;
  
    QStringList needNamespace = m_scope.toStringList();
    
    bool foundChild = true;
    while(!needNamespace.isEmpty() && foundChild) {
      foundChild = false;
      
      foreach(DUContext* child, context->childContexts()) {
        if(child->localScopeIdentifier().toString() == needNamespace.first() && child->type() == DUContext::Namespace && child->range().start < m_insertBefore) {
          context = child;
          foundChild = true;
          needNamespace.pop_front();
          break;
        }
      }
    }
  
    m_context = context;
    m_scope = QualifiedIdentifier(needNamespace.join("::"));
}

QString KDevelop::SourceCodeInsertion::applySubScope(QString decl) const {
  QString ret;
  QString scopeType = "namespace";
  QString scopeClose;

  if(m_context && m_context->type() == DUContext::Class) {
    scopeType = "struct";
    scopeClose =  ";";
  }
  
  foreach(QString scope, m_scope.toStringList())
    ret += scopeType + " " + scope + " {\n";
  
  ret += decl;

  foreach(QString scope, m_scope.toStringList())
    ret += "}" + scopeClose + "\n";
  
  return ret;
}

void KDevelop::SourceCodeInsertion::setAccess(KDevelop::Declaration::AccessPolicy access) {
  m_access = access;
}

KDevelop::SourceCodeInsertion::SourceCodeInsertion(KDevelop::TopDUContext* topContext) : m_topContext(topContext), m_access(Declaration::Public), m_context(topContext) {
  m_codeRepresentation = KDevelop::createCodeRepresentation(m_topContext->url());
}

KDevelop::SourceCodeInsertion::~SourceCodeInsertion() {
  delete m_codeRepresentation;
}

QString KDevelop::SourceCodeInsertion::accessString() const {
  switch(m_access) {
    case KDevelop::Declaration::Public:
      return "public";
    case KDevelop::Declaration::Protected:
      return "protected";
    case KDevelop::Declaration::Private:
      return "private";
  }
  return QString();
}

QString KDevelop::SourceCodeInsertion::indentation() const {
  if(!m_codeRepresentation || !m_context || m_context->localDeclarations(m_topContext).size() == 0) {
    kDebug() << "cannot do indentation";
    return QString();
  }
  
  foreach(Declaration* decl, m_context->localDeclarations(m_topContext)) {
    if(decl->range().isEmpty() || decl->range().start.column == 0)
      continue; //Skip declarations with empty range, that were expanded from macros
    int spaces = 0;
    
    QString textLine = m_codeRepresentation->line(decl->range().start.line);
    
    for(int a = 0; a < textLine.size(); ++a) {
      if(textLine[a].isSpace())
        ++spaces;
      else
        break;
    }
    
    return textLine.left(spaces);
  }
  
  return QString();
}

QString KDevelop::SourceCodeInsertion::applyIndentation(QString decl) const {
  QStringList lines = decl.split('\n');
  QString ind = indentation();
  QStringList ret;
  foreach(QString line, lines) {
    if(!line.isEmpty())
      ret << ind + line;
    else
      ret << line;
  }
  return ret.join("\n");;
}

QString makeSignatureString(QList<SourceCodeInsertion::SignatureItem> signature, DUContext* context) {
  QString ret;
  foreach(SourceCodeInsertion::SignatureItem item, signature) {
    if(!ret.isEmpty())
      ret += ", ";
    AbstractType::Ptr type = TypeUtils::removeConstants(item.type);
    ret += Cpp::shortenedTypeString(type, 100000, context->scopeIdentifier(true));
    
    if(!item.name.isEmpty())
      ret += " " + item.name;
  }
  return ret;
}

bool KDevelop::SourceCodeInsertion::insertFunctionDeclaration(KDevelop::Identifier name, AbstractType::Ptr returnType, QList<SignatureItem> signature, KDevelop::Declaration::AccessPolicy policy, bool isConstant) {
  if(!m_context)
    return false;
  
  returnType = TypeUtils::removeConstants(returnType);
  
  QString decl = Cpp::shortenedTypeString(returnType, 100000, m_context->scopeIdentifier(true)) + " " + name.toString() + "(" + makeSignatureString(signature, m_context) + ")";
  
  if(isConstant)
    decl += " const";
  
  decl += ";\n";
  
  InsertionPoint insertion = findInsertionPoint(m_access, Variable);
  
  decl = applyIndentation(applySubScope(insertion.prefix +decl));
  
  return m_changeSet.addChange(DocumentChange(m_context->url(), SimpleRange(insertion.line, 0, insertion.line, 0), QString(), decl));
}

bool KDevelop::SourceCodeInsertion::insertVariableDeclaration(KDevelop::Identifier name, KDevelop::AbstractType::Ptr type) {

  if(!m_context)
    return false;
  
  type = TypeUtils::removeConstants(type);
  
  QString decl = Cpp::shortenedTypeString(type, 100000, m_context->scopeIdentifier(true)) + " " + name.toString() + ";\n";
  
  InsertionPoint insertion = findInsertionPoint(m_access, Variable);
  
  decl = applyIndentation(applySubScope(insertion.prefix + decl));
  
  return m_changeSet.addChange(DocumentChange(m_context->url(), SimpleRange(insertion.line, 0, insertion.line, 0), QString(), decl));
}

SourceCodeInsertion::InsertionPoint SourceCodeInsertion::findInsertionPoint(KDevelop::Declaration::AccessPolicy policy, InsertionKind kind) const {
  InsertionPoint ret;
  ret.line = m_context->range().end.line;
  
    bool behindExistingItem = false;
    foreach(Declaration* decl, m_context->localDeclarations()) {
      ClassMemberDeclaration* classMem = dynamic_cast<ClassMemberDeclaration*>(decl);
      if(m_context->type() != DUContext::Class || (classMem && classMem->accessPolicy() == m_access) || m_access == KDevelop::Declaration::Public) {
        
        Cpp::QtFunctionDeclaration* qtFunction = dynamic_cast<Cpp::QtFunctionDeclaration*>(decl);
        
        if( (kind == Slot && qtFunction && qtFunction->isSlot()) ||
            (kind == Function && dynamic_cast<AbstractFunctionDeclaration*>(decl)) ||
            (kind == Variable && decl->kind() == Declaration::Instance && !dynamic_cast<AbstractFunctionDeclaration*>(decl)) ) {
          behindExistingItem = true;
          ret.line = decl->range().end.line+1;
        }
      }
    }
    
    if(!behindExistingItem) {
      Cpp::ClassDeclaration* classDecl = dynamic_cast<Cpp::ClassDeclaration*>(m_context->owner());
      if(kind != Slot && m_access == Declaration::Public && classDecl && classDecl->classType() == Cpp::ClassDeclarationData::Struct) {
        //Nothing to do, we can just insert into a struct if it should be public
      }else{
        ret.prefix = accessString();
        if(kind == Slot)
        ret.prefix +=  " slots";
        ret.prefix += ":\n";
      }
    }
    
    
  return ret;
}

bool Cpp::SourceCodeInsertion::insertSlot(QString name, QString normalizedSignature) {
    if(!m_context || !m_codeRepresentation)
      return false;
  
    InsertionPoint insertion = findInsertionPoint(m_access, Slot);
    
    QString add = insertion.prefix;
    
    QString sig;
    add += "void " + name + "(" + normalizedSignature + ");\n";
    
    if(insertion.line > m_codeRepresentation->lines())
      return false;

    add = applyIndentation(add);
    
    return m_changeSet.addChange(DocumentChange(m_context->url(), SimpleRange(insertion.line, 0, insertion.line, 0), QString(), add));
}

bool Cpp::SourceCodeInsertion::insertForwardDeclaration(KDevelop::Declaration* decl) {
  setSubScope(decl->context()->scopeIdentifier(true));
  
  if(!m_context) {
    kDebug() << "no context";
    return false;
  }
  
    QString forwardDeclaration;
    if(decl->type<KDevelop::EnumerationType>()) {
      forwardDeclaration = "enum " + decl->identifier().toString() + ";\n";
    }else if(decl->isTypeAlias()) {
      if(!decl->abstractType()) {
        kDebug() << "no type";
        return false;
      }
      
      forwardDeclaration = "typedef " + decl->abstractType()->toString() + " " + decl->identifier().toString() + ";\n";
    }else{
      DUContext* templateContext = getTemplateContext(decl);
      if(templateContext) {
        forwardDeclaration += "template<";
        bool first = true;
        foreach(Declaration* _paramDecl, templateContext->localDeclarations()) {
          TemplateParameterDeclaration* paramDecl = dynamic_cast<TemplateParameterDeclaration*>(_paramDecl);
          if(!paramDecl)
            continue;
          if(!first) {
            forwardDeclaration += ", ";
          }else{
            first = false;
          }
          
          CppTemplateParameterType::Ptr templParamType = paramDecl->type<CppTemplateParameterType>();
          if(templParamType) {
            forwardDeclaration += "class ";
          }else if(paramDecl->abstractType()) {
            forwardDeclaration += paramDecl->abstractType()->toString() + " ";
          }
          
          forwardDeclaration += paramDecl->identifier().toString();
          
          if(!paramDecl->defaultParameter().isEmpty()) {
            forwardDeclaration += " = " + paramDecl->defaultParameter().toString();
          }
        }
        
        forwardDeclaration += " >\n";
      }
      forwardDeclaration += "class " + decl->identifier().toString() + ";\n";
    }
    
    //Put declarations to the end, and namespaces to the begin
    KTextEditor::Cursor position;
    
    if(!m_scope.isEmpty() || m_context->range().end > m_insertBefore) {
      //To the begin
      position = m_context->range().start.textCursor();
      
      if(m_context->type() == DUContext::Namespace) {
          position += KTextEditor::Cursor(0, 1); //Skip over the opening '{' paren
        
        //Put the newline to the beginning instead of the end
        forwardDeclaration = "\n" + forwardDeclaration;
        if(forwardDeclaration.endsWith("\n"))
          forwardDeclaration = forwardDeclaration.left(forwardDeclaration.length()-1);
      }
    } else{
      //To the end
      position = m_context->range().end.textCursor() - KTextEditor::Cursor(0, 1);
    }
    int firstValidLine = firstValidCodeLineBefore(position.line());
    if(firstValidLine > position.line() && m_context == m_topContext && firstValidLine < m_insertBefore.line) {
      position.setLine(firstValidLine);
      position.setColumn(0);
    }
    
    forwardDeclaration = applySubScope(forwardDeclaration);
    
    kDebug() << "inserting at" << position << forwardDeclaration;
    
    return m_changeSet.addChange(DocumentChange(m_context->url(), SimpleRange(position.line(), position.column(), position.line(), position.column()), QString(), forwardDeclaration));
}

Cpp::SourceCodeInsertion::SourceCodeInsertion(TopDUContext* topContext) : KDevelop::SourceCodeInsertion(topContext){

}
