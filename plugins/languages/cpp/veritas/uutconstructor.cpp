/*
* KDevelop xUnit integration
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#include "uutconstructor.h"
#include "documentaccess.h"

// kdevplatform
#include <language/duchain/declaration.h>
#include <language/editor/simplecursor.h>
#include <language/duchain/duchain.h>
#include <language/duchain/use.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/delayedtype.h>

#include "../cppduchain/expressionparser.h"
#include "../cppduchain/expressionevaluationresult.h"

#include <ktexteditor/smartrange.h>

using Veritas::UUTConstructor;
using Veritas::ClassSkeleton;
using Veritas::MethodSkeleton;
using KTextEditor::SmartRange;
using namespace KDevelop;
using Cpp::ExpressionParser;
using Cpp::ExpressionEvaluationResult;

namespace
{

/*! Debugger helper for types */
void printTypeInfoFor(Declaration* decl)
{
    AbstractType::Ptr atype = decl->type<AbstractType>();
    if (atype) {
        kDebug() << "Not an Unresolved type [" << atype->toString()
                 << " " << decl->toString() << "]";
    } else {
        kDebug() << "Failed to fetch type for " << decl->toString();
    }
}

/*! Serialize @param type to string. Remove any prefixes. */
QString cleanTypeNameFor(DelayedType::Ptr type)
{
    QString strType = type->toString();
    if (strType.startsWith("<unresolved> ")) {
        strType = strType.split(' ').last();
    }
    return strType;
}

/*! Fetch the declaration associated with @param use */
Declaration* declarationForUse(const Use* use, DUContext* ctx)
{
    TopDUContext* top = ctx->topContext();
    int declarationId = use->m_declarationIndex;
    return top->usedDeclarationForIndex(declarationId);
}

DelayedType::Ptr isUnresolvedType(Declaration* variable)
{
    DelayedType::Ptr type = variable->type<DelayedType>();
    if (!type || type->kind() != DelayedType::Unresolved) {
        printTypeInfoFor(variable);
        type = 0;
    }
    return type;
}

void printAllDeclarations(DUContext* ctx)
{
    kDebug() << "";
    foreach(Declaration* dcl, ctx->localDeclarations()) {
        kDebug() << dcl->toString();
    }
    kDebug() << "";
}

} // end anonymous namespace

void UUTConstructor::printUseInfo(int useId, const Use* use, DUContext* ctx)
{
    Declaration* decl = declarationForUse(use,ctx);
    if (!decl) return;
    kDebug() << "decl    " << decl->toString();
    KUrl url(decl->url().str());
    kDebug() << "useTxt  " << m_docAccess->text(url, use->m_range);
}

ClassSkeleton UUTConstructor::morph(Declaration* variable)
{
    Q_ASSERT(variable); Q_ASSERT(m_docAccess);
    DUChainReadLocker lock(DUChain::lock());

    ClassSkeleton cs;
    DelayedType::Ptr varType = isUnresolvedType(variable);
    if (!varType) return cs;
    cs.setName(cleanTypeNameFor(varType));

    DUContext* ctx = variable->context();
    printAllDeclarations(ctx);
    constructMethodsFor(ctx, variable, cs);
    return cs;
}

void UUTConstructor::constructMethodsFor(DUContext* ctx, Declaration* classToMorph, ClassSkeleton& cs)
{
    const Use* uses = ctx->uses();
    for(int i=0; i<ctx->usesCount(); i++, uses++) {
        Declaration* decl = declarationForUse(uses, ctx);
        printUseInfo(i, uses, ctx);
        if (decl == classToMorph) {
            MethodSkeleton ms = createMethod(decl, uses, ctx);
            if (!ms.isEmpty()) {
              cs.addMethod(ms);
            }
        }
    }
    foreach(DUContext* child, ctx->childContexts()) {
        constructMethodsFor(child, classToMorph, cs);
    }
}


UUTConstructor::UUTConstructor()
  : m_docAccess(0)
{}

UUTConstructor::~UUTConstructor()
{}

void UUTConstructor::setDocumentAccess(Veritas::DocumentAccess* docAccess)
{
    Q_ASSERT(!m_docAccess);
    m_docAccess = docAccess;
    Q_ASSERT(m_docAccess);
}


QString UUTConstructor::enoughText(const KUrl& url, const KDevelop::SimpleCursor& cursor) const
{
    SimpleCursor ahead(cursor.line+4, 0); // though luck if you smear a use over more than 3 lines.
    //SimpleCursor ahead(cursor.line, cursor.column+10);
    SimpleRange farEnough(cursor, ahead);
    QString txt = m_docAccess->text(url, farEnough);
    kDebug() << txt;
    return txt;
}

namespace
{
/*! remove spaces, newlines and stuff after the first semicolon.
    Also get rid of first '.' or '->' */
void normalize(QString& text)
{
    kDebug() << QString("before ``%1''").arg(text);
    text.remove(QRegExp("[ \r\n\t]"));
    int pos = text.indexOf(";");
    if (pos == -1) {
        text.clear();
        kDebug() << QString("after ``%1''").arg(text);
        return;
    }
    text = text.left(pos);
    if (text.startsWith('.')) {
        text = text.mid(1, -1);
    } else if (text.startsWith("->")) {
        text = text.mid(2, -1);
    } else {
        text.clear();
    }
    kDebug() << QString("after ``%1''").arg(text);
}

QString extractMethodName(QString& text)
{
    kDebug() << text;
    int firstBrace = text.indexOf("(");
    QString methodName = text.left(firstBrace);
    text = text.mid(firstBrace);
    return methodName;
}

QList<int> topLevelCommaPositions(const QString& text)
{
    QStack<bool> braces;
    QList<int> result;
    if (!text.contains(',')) return result;
    QString text_ = text.mid(1, text.count()-1);
    for (int i=0; i<text_.count(); i++) {
        QChar c = text_[i];
        if (c == ',') {
            if (braces.isEmpty()) result << i;
        } else if (c == '(') {
            braces.push(true);
        } else if (c == ')' && !braces.isEmpty()) {
            braces.pop();
        }
    }
    return result;
}

QString serializeTypeFromExpression(const QString& expr, DUContext* ctx)
{
  ExpressionParser* ep = new ExpressionParser;
  DUContextPointer p(ctx);
  TopDUContext* top = ctx->topContext();
  ExpressionEvaluationResult er = ep->evaluateType(expr.toLatin1(), p, top);
  delete ep;
  if (!er.type) {
    kDebug() << "failed to grab arguments";
    return QString();
  }
  if (er.type.type()->whichType() == AbstractType::TypeFunction) {
    TypePtr<FunctionType> f = er.type.type().cast<FunctionType>();
    Q_ASSERT(f); Q_ASSERT(f->returnType());
    return f->returnType()->toString();
  } else {
    return er.type.type()->toString();
  }
}

QString extractArguments(QString& text, DUContext* ctx)
{
    kDebug() << text;
    QList<int> commas = topLevelCommaPositions(text);
    kDebug() << "COMMAS " << commas;
    if (commas.isEmpty()) {
        int lastBrace = text.indexOf(')');
        QString type = serializeTypeFromExpression(text.mid(1, lastBrace), ctx);
        if (type.isEmpty()) return QString();
        return QString("(%1)").arg(type);
    } else {
        QString args("(");
        int prev = 1;
        foreach(int i, commas) {
            i = i+1;
            QString expr = text.mid(prev, i);
            args += serializeTypeFromExpression(expr, ctx) + ", ";
            prev = i+1;
        }
        int lastBrace = text.indexOf(')');
        args += serializeTypeFromExpression(text.mid(prev, lastBrace), ctx);
        args += ')';
        return args;
    }
}

} // end anonymous namespace

MethodSkeleton UUTConstructor::createMethod(Declaration* clazz, const Use* use, DUContext* ctx)
{
    KUrl doc(clazz->url().str());
    MethodSkeleton ms;
    QString text = enoughText(doc, use->m_range.end);
    normalize(text);
    if (text.isEmpty()) {
        kDebug() << "Failed to parser\n" << enoughText(doc, use->m_range.end);
        return ms;
    }
    if (text.contains("(") && text.contains(")")) { // function member use
        ms.setName(extractMethodName(text));
        if (!text.startsWith("()")) {
            ms.setArguments(extractArguments(text, ctx));
        }
        ms.setBody("// GENERATED");
    } else { // data member use. do nothing for now.
    }
    return ms;
}

