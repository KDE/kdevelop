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

using namespace KDevelop;

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
}

void KDevelop::SourceCodeInsertion::setAccess(KDevelop::Declaration::AccessPolicy access) {
  m_access = access;
}

KDevelop::SourceCodeInsertion::SourceCodeInsertion(KDevelop::TopDUContext* topContext) : m_topContext(topContext), m_access(Declaration::Public), m_context(0) {
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

bool KDevelop::SourceCodeInsertion::insertVariableDeclaration(KDevelop::Identifier /*name*/, KDevelop::AbstractType::Ptr /*type*/) {

  return false;
}

bool Cpp::SourceCodeInsertion::insertSlot(QString name, QString normalizedSignature) {
    if(!m_context || !m_codeRepresentation)
      return false;
  
    QString indent = indentation();
    
    QStringList text = m_codeRepresentation->text().split('\n');
    int targetLine = m_context->range().end.line;
    bool behindExistingSlot = false;
    foreach(Declaration* decl, m_context->localDeclarations()) {
      if(Cpp::QtFunctionDeclaration* qtFunction = dynamic_cast<Cpp::QtFunctionDeclaration*>(decl)) {
        if(qtFunction->isSlot() && qtFunction->accessPolicy() == m_access) {
          //Prefer putting the declaration behind existing slots
          behindExistingSlot = true;
          targetLine = qtFunction->range().end.line+1;
        }
      }
    }
    
    QString add;
    if(!behindExistingSlot)
      add = indentation() + accessString() +  " slots:\n";
    
    QString sig;
    sig = "(" + normalizedSignature + ")";
    add += indentation() + "void " + name + sig + ";\n";
    
    if(targetLine > text.size())
      return false;
    
    return m_changeSet.addChange(DocumentChangePointer( new DocumentChange(m_context->url(), SimpleRange(targetLine, 0, targetLine, 0), QString(), add) ));
}

Cpp::SourceCodeInsertion::SourceCodeInsertion(TopDUContext* topContext) : KDevelop::SourceCodeInsertion(topContext){

}
