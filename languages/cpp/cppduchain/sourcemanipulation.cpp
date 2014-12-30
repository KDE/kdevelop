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
#include "templateparameterdeclaration.h"
#include "debug.h"
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/stringhelpers.h>

using namespace KDevelop;

///Makes sure the line is not in a comment, moving it behind if needed. Just does very simple matching, should be ok for header copyright-notices and such.
int KDevelop::SourceCodeInsertion::firstValidCodeLineBefore(int lineNumber) const {
  if(lineNumber == -1)
    lineNumber = 1000000;

  if(lineNumber > 300)
    lineNumber = 300; //Don't do too much processing

    int checkLines = m_codeRepresentation->lines() < lineNumber ? m_codeRepresentation->lines() : lineNumber;

    int chosen = -1;

    //Add some whitespace so we always have some comment clearing done, in every line
    QString allText = "         \n";
    // we've added a line, hence increase our limit as well
    checkLines += 1;
    for(int a = 0; a < checkLines; ++a)
      allText += m_codeRepresentation->line(a) + "         \n";
    allText = KDevelop::clearComments(allText, '$');

    QStringList lines = allText.split('\n');
    if(lines.count() < checkLines)
      checkLines = lines.count();

    int lastDefine = -1;
    int lastComment = -1;

    for(int a = 0; a < checkLines; ++a) {
      if(lines.at(a).startsWith('$')) {
        chosen = -1;
        lastComment = a;
        continue;
      }
      QString trimmedLine = lines.at(a).trimmed();
      if(trimmedLine.startsWith('#')) {
        lastDefine = a;
        chosen = -1;
        continue;
      }

      if(trimmedLine.isEmpty() && chosen == -1)
        chosen = a;
      if(!trimmedLine.isEmpty())
        break;
    }

  if(chosen == -1 && lastDefine != -1) {
    chosen = lastDefine + 1;
  }
  if (chosen == -1 && lastComment != -1) {
    chosen = lastComment + 1;
  }

  if(chosen != -1) {
    // if chosen == 0 we can only add at the start of the document
    // NOTE: chosen must be reduced by one since we add one line
    // at the beginning, see above (allText = ...)
    return qMax(0, chosen - 1);
  } else
    return lineNumber;
}

//Re-indents the code so the leftmost line starts at zero
QString zeroIndentation(QString str, int fromLine = 0) {
  QStringList lines = str.split('\n');
  QStringList ret;

  if(fromLine < lines.size()) {
    ret = lines.mid(0, fromLine);
    lines = lines.mid(fromLine);
  }


  QRegExp nonWhiteSpace("\\S");
  int minLineStart = 10000;
  foreach(const QString& line, lines) {
    int lineStart = line.indexOf(nonWhiteSpace);
    if(lineStart < minLineStart)
      minLineStart = lineStart;
  }

  foreach(const QString& line, lines)
    ret << line.mid(minLineStart);

  return ret.join("\n");
}

KDevelop::DocumentChangeSet& KDevelop::SourceCodeInsertion::changes() {
  return m_changeSet;
}

void KDevelop::SourceCodeInsertion::setInsertBefore(KTextEditor::Cursor position) {
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
        qCDebug(CPPDUCHAIN) << "checking child" << child->localScopeIdentifier().toString() << "against" << needNamespace.first();
        if(child->localScopeIdentifier().toString() == needNamespace.first()
            && child->type() == DUContext::Namespace
            && (child->rangeInCurrentRevision().start() < m_insertBefore || !m_insertBefore.isValid())) {
          qCDebug(CPPDUCHAIN) << "taking";
          context = child;
          foundChild = true;
          needNamespace.pop_front();
          break;
        }
      }
    }

    m_context = context;
    m_scope  = Cpp::stripPrefixes(context, QualifiedIdentifier(needNamespace.join("::")));
}

QString KDevelop::SourceCodeInsertion::applySubScope(QString decl) const {
  QString ret;
  QString scopeType = "namespace";
  QString scopeClose;

  if(m_context && m_context->type() == DUContext::Class) {
    scopeType = "struct";
    scopeClose =  ";";
  }

  foreach(const QString& scope, m_scope.toStringList())
    ret += scopeType + " " + scope + " {\n";

  ret += decl;

  ret += QString("}" + scopeClose + "\n").repeated(m_scope.count());

  return ret;
}

void KDevelop::SourceCodeInsertion::setAccess(KDevelop::Declaration::AccessPolicy access) {
  m_access = access;
}

KDevelop::SourceCodeInsertion::SourceCodeInsertion(KDevelop::TopDUContext* topContext)
  : m_context(topContext), m_access(Declaration::Public), m_topContext(topContext),
    m_codeRepresentation(KDevelop::createCodeRepresentation(m_topContext->url()))
{
  if(m_topContext->parsingEnvironmentFile() && m_topContext->parsingEnvironmentFile()->isProxyContext()) {
    qCWarning(CPPDUCHAIN) << "source-code manipulation on proxy-context is wrong!!!" << m_topContext->url().toUrl();
  }
  m_insertBefore = KTextEditor::Cursor::invalid();
}

KDevelop::SourceCodeInsertion::~SourceCodeInsertion() {
}

QString KDevelop::SourceCodeInsertion::accessString() const {
  switch(m_access) {
    case KDevelop::Declaration::Public:
      return "public";
    case KDevelop::Declaration::Protected:
      return "protected";
    case KDevelop::Declaration::Private:
      return "private";
    default:
      return QString();
  }
}

QString KDevelop::SourceCodeInsertion::indentation() const {
  if(!m_codeRepresentation || !m_context || m_context->localDeclarations(m_topContext).size() == 0) {
    qCDebug(CPPDUCHAIN) << "cannot do indentation";
    return QString();
  }

  foreach(Declaration* decl, m_context->localDeclarations(m_topContext)) {
    if(decl->range().isEmpty() || decl->range().start.column == 0)
      continue; //Skip declarations with empty range, that were expanded from macros
    int spaces = 0;

    QString textLine = m_codeRepresentation->line(decl->range().start.line);

    for(int a = 0; a < textLine.size(); ++a) {
      if(textLine.at(a).isSpace())
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
  foreach(const QString& line, lines) {
    if(!line.isEmpty())
      ret << ind + line;
    else
      ret << line;
  }
  return ret.join("\n");;
}

QString makeSignatureString(QList<SourceCodeInsertion::SignatureItem> signature, DUContext* context) {
  QString ret;
  foreach(const SourceCodeInsertion::SignatureItem& item, signature) {
    if(!ret.isEmpty())
      ret += ", ";
    AbstractType::Ptr type = TypeUtils::removeConstants(item.type, context->topContext());
    ret += Cpp::simplifiedTypeString(type, context);

    if(!item.name.isEmpty())
      ret += " " + item.name;
  }
  return ret;
}

KTextEditor::Range SourceCodeInsertion::insertionRange(int line)
{
  if(line == 0 || !m_codeRepresentation)
    return KTextEditor::Range(line, 0, line, 0);
  else
  {
    KTextEditor::Range range(line-1, m_codeRepresentation->line(line-1).size(), line-1, m_codeRepresentation->line(line-1).size());
    //If the context finishes on that line, then this will need adjusting
    if(!m_context->rangeInCurrentRevision().contains(range))
    {
      range.start() = m_context->rangeInCurrentRevision().end();
      if(range.start().column() > 0)
        range.start() = range.start() - KTextEditor::Cursor(0, 1);
      range.end() = range.start();
    }
    return range;
  }
}

bool KDevelop::SourceCodeInsertion::insertFunctionDeclaration(KDevelop::Identifier name, AbstractType::Ptr returnType, QList<SignatureItem> signature, bool isConstant, QString body) {
  if(!m_context)
    return false;

  returnType = TypeUtils::removeConstants(returnType, m_topContext);

  QString decl = (returnType ? (Cpp::simplifiedTypeString(returnType, m_context) + " ") : QString()) + name.toString() + "(" + makeSignatureString(signature, m_context) + ")";

  if(isConstant)
    decl += " const";

  if(body.isEmpty())
    decl += ";";
  else {
    if (!body.startsWith(' ') && !body.startsWith('\n')) {
      decl += " ";
    }
    decl += zeroIndentation(body);
  }

  InsertionPoint insertion = findInsertionPoint(m_access, Function);

  decl = "\n" + applyIndentation(applySubScope(insertion.prefix +decl));

  return m_changeSet.addChange(DocumentChange(m_context->url(), insertionRange(insertion.line), QString(), decl));
}

bool KDevelop::SourceCodeInsertion::insertVariableDeclaration(KDevelop::Identifier name, KDevelop::AbstractType::Ptr type) {

  if(!m_context)
    return false;

  type = TypeUtils::removeConstants(type, m_topContext);

  QString decl = Cpp::simplifiedTypeString(type, m_context) + " " + name.toString() + ";";

  InsertionPoint insertion = findInsertionPoint(m_access, Variable);

  decl = "\n" + applyIndentation(applySubScope(insertion.prefix + decl));

  return m_changeSet.addChange(DocumentChange(m_context->url(), insertionRange(insertion.line), QString(), decl));
}



KTextEditor::Cursor SourceCodeInsertion::end() const
{
  KTextEditor::Cursor ret = m_context->rangeInCurrentRevision().end();
  if(m_codeRepresentation && m_codeRepresentation->lines() && dynamic_cast<TopDUContext*>(m_context)) {
    ret.setLine(m_codeRepresentation->lines()-1);
    ret.setColumn(m_codeRepresentation->line(ret.line()).size());
  }
  return ret;

}

SourceCodeInsertion::InsertionPoint SourceCodeInsertion::findInsertionPoint(KDevelop::Declaration::AccessPolicy policy, InsertionKind kind) const {
  Q_UNUSED(policy);
  InsertionPoint ret;
  ret.line = end().line();

    bool behindExistingItem = false;

    //Try twice, in the second run, only match the "access"
    for(int anyMatch = 0; anyMatch <= 1 && !behindExistingItem; ++anyMatch) {

      foreach(Declaration* decl, m_context->localDeclarations()) {
        ClassMemberDeclaration* classMem = dynamic_cast<ClassMemberDeclaration*>(decl);
        if(m_context->type() != DUContext::Class || (classMem && classMem->accessPolicy() == m_access)) {

          Cpp::QtFunctionDeclaration* qtFunction = dynamic_cast<Cpp::QtFunctionDeclaration*>(decl);

          if( (kind != Slot && anyMatch) || //Only allow anyMatch if not searching a slot, since else it may end up in a wrong section, not being a slot at all
              (kind == Slot && qtFunction && qtFunction->isSlot()) ||
              (kind == Function && dynamic_cast<AbstractFunctionDeclaration*>(decl)) ||
              (kind == Variable && decl->kind() == Declaration::Instance && !dynamic_cast<AbstractFunctionDeclaration*>(decl)) ) {
            behindExistingItem = true;
            ret.line = decl->range().end.line+1;
          if(decl->internalContext())
            ret.line = decl->internalContext()->range().end.line+1;
          }
        }
      }
    }
    qCDebug(CPPDUCHAIN) << ret.line << m_context->scopeIdentifier(true) << m_context->rangeInCurrentRevision() << behindExistingItem << m_context->url().toUrl() << m_context->parentContext();
    qCDebug(CPPDUCHAIN) << "is proxy:" << m_context->topContext()->parsingEnvironmentFile()->isProxyContext() << "count of declarations:" << m_context->topContext()->localDeclarations().size();

    if(!behindExistingItem) {
      ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration*>(m_context->owner());
      if(kind != Slot && m_access == Declaration::Public && classDecl && classDecl->classType() == ClassDeclarationData::Struct) {
        //Nothing to do, we can just insert into a struct if it should be public
      }else if(m_context->type() == DUContext::Class) {
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
    add += "void " + name + "(" + normalizedSignature + ");";

    if(insertion.line > m_codeRepresentation->lines())
      return false;

    add = "\n" + applyIndentation(add);

    return m_changeSet.addChange(DocumentChange(m_context->url(), insertionRange(insertion.line), QString(), add));
}

bool Cpp::SourceCodeInsertion::insertForwardDeclaration(KDevelop::Declaration* decl) {
  setSubScope(decl->context()->scopeIdentifier(true));

  if(!m_context) {
    qCDebug(CPPDUCHAIN) << "no context";
    return false;
  }

    QString forwardDeclaration;
    if(decl->kind() == Declaration::Instance) {
      //A simple variable declaration
      forwardDeclaration = "extern " + Cpp::simplifiedTypeString(decl->abstractType(), m_context) + " " + decl->identifier().toString() + ";";
    }else if(decl->type<KDevelop::EnumerationType>()) {
      forwardDeclaration = "enum " + decl->identifier().toString() + ";";
    }else if(decl->isTypeAlias()) {
      if(!decl->abstractType()) {
        qCDebug(CPPDUCHAIN) << "no type";
        return false;
      }

      forwardDeclaration = "typedef " + Cpp::simplifiedTypeString(decl->abstractType(), m_context) + " " + decl->identifier().toString() + ";";
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
            forwardDeclaration += Cpp::simplifiedTypeString(paramDecl->abstractType(), m_context) + " ";
          }

          forwardDeclaration += paramDecl->identifier().toString();

          if(!paramDecl->defaultParameter().isEmpty()) {
            forwardDeclaration += " = " + paramDecl->defaultParameter().toString();
          }
        }

        forwardDeclaration += " >\n";
      }

      ClassDeclaration * classDecl = dynamic_cast<ClassDeclaration *>(decl);
      if(classDecl) {
        forwardDeclaration += classDecl->toString() + ";";
      }else{
        forwardDeclaration += "class " + decl->identifier().toString() + ";";
      }
    }

    //Put declarations to the end, and namespaces to the begin
    KTextEditor::Cursor position;

    bool needNewLine = true;

    if(!m_scope.isEmpty() || (m_insertBefore.isValid() && m_context->rangeInCurrentRevision().end() > m_insertBefore)) {
      //To the begin
      position = m_context->rangeInCurrentRevision().start();

      if(m_context->type() == DUContext::Namespace) {
          position += KTextEditor::Cursor(0, 1); //Skip over the opening '{' paren

        //Put the newline to the beginning instead of the end
        forwardDeclaration = "\n" + forwardDeclaration;
        if(forwardDeclaration.endsWith("\n"))
          forwardDeclaration = forwardDeclaration.left(forwardDeclaration.length()-1);
      }
    } else{
      //To the end

      position = end();
      if(position.column() != 0 && m_context->type() == DUContext::Namespace) {
        position -= KTextEditor::Cursor(0, 1);
        forwardDeclaration += "\n";
        needNewLine = false;
      }
    }
    int firstValidLine = firstValidCodeLineBefore(m_insertBefore.line());
    if(firstValidLine > position.line() && m_context == m_topContext && (!m_insertBefore.isValid() || firstValidLine < m_insertBefore.line())) {
      position.setLine(firstValidLine);
      position.setColumn(0);
    }

    forwardDeclaration = applySubScope(forwardDeclaration);
    if(needNewLine)
      forwardDeclaration = "\n" + forwardDeclaration;

    qCDebug(CPPDUCHAIN) << "inserting at" << position << forwardDeclaration;

    return m_changeSet.addChange(DocumentChange(m_context->url(), KTextEditor::Range(position.line(), position.column(), position.line(), position.column()), QString(), forwardDeclaration));
}

Cpp::SourceCodeInsertion::SourceCodeInsertion(TopDUContext* topContext) : KDevelop::SourceCodeInsertion(topContext){
    m_changeSet.setFormatPolicy(KDevelop::DocumentChangeSet::AutoFormatChanges);
}
