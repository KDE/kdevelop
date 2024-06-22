/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "sourcemanipulation.h"

#include <interfaces/icore.h>
#include <interfaces/isourceformattercontroller.h>

#include <language/codegen/coderepresentation.h>

#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/types/enumeratortype.h>
#include <language/duchain/types/functiontype.h>

#include "codegenhelper.h"
#include "adaptsignatureaction.h"
#include "util/clangdebug.h"

#include <QRegularExpression>

using namespace KDevelop;

namespace
{
QualifiedIdentifier stripPrefixes(const DUContextPointer& ctx, const QualifiedIdentifier& id)
{
    if (!ctx) {
        return id;
    }

    auto imports = ctx->fullyApplyAliases({}, ctx->topContext());
    if (imports.contains(id)) {
        return {}; /// The id is a namespace that is imported into the current context
    }

    auto basicDecls = ctx->findDeclarations(id, CursorInRevision::invalid(), {}, nullptr,
                                            (DUContext::SearchFlags)(DUContext::NoSelfLookUp | DUContext::NoFiltering));

    if (basicDecls.isEmpty()) {
        return id;
    }

    auto newId = id.mid(1);
    auto result = id;
    while (!newId.isEmpty()) {
        auto foundDecls
            = ctx->findDeclarations(newId, CursorInRevision::invalid(), {}, nullptr,
                                    (DUContext::SearchFlags)(DUContext::NoSelfLookUp | DUContext::NoFiltering));

        if (foundDecls == basicDecls) {
            result = newId; // must continue to find the shortest possible identifier
            // esp. for cases where nested namespaces are used (e.g. using namespace a::b::c;)
            newId = newId.mid(1);
        }
    }

    return result;
}

// Re-indents the code so the leftmost line starts at zero
QString zeroIndentation(const QString& str, int fromLine = 0)
{
    QStringList lines = str.split(QLatin1Char('\n'));
    QStringList ret;

    if (fromLine < lines.size()) {
        ret = lines.mid(0, fromLine);
        lines = lines.mid(fromLine);
    }

    static const QRegularExpression nonWhiteSpace(QStringLiteral("\\S"));
    int minLineStart = 10000;
    for (const auto& line : std::as_const(lines)) {
        int lineStart = line.indexOf(nonWhiteSpace);
        if (lineStart < minLineStart) {
            minLineStart = lineStart;
        }
    }

    ret.reserve(ret.size() + lines.size());
    for (const auto& line : std::as_const(lines)) {
        ret << line.mid(minLineStart);
    }

    return ret.join(QLatin1Char('\n'));
}
}

DocumentChangeSet SourceCodeInsertion::changes()
{
    return m_changeSet;
}

void SourceCodeInsertion::setSubScope(const QualifiedIdentifier& scope)
{
    m_scope = scope;

    if (!m_context) {
        return;
    }

    QStringList needNamespace = m_scope.toStringList();

    bool foundChild = true;
    while (!needNamespace.isEmpty() && foundChild) {
        foundChild = false;

        const auto childContexts = m_context->childContexts();
        for (DUContext* child : childContexts) {
            clangDebug() << "checking child" << child->localScopeIdentifier().toString() << "against"
                     << needNamespace.first();
            if (child->localScopeIdentifier().toString() == needNamespace.first() && child->type() == DUContext::Namespace) {
                clangDebug() << "taking";
                m_context = child;
                foundChild = true;
                needNamespace.pop_front();
                break;
            }
        }
    }

    m_scope = stripPrefixes(m_context, QualifiedIdentifier(needNamespace.join(QLatin1String("::"))));
}

QString SourceCodeInsertion::applySubScope(const QString& decl) const
{
    if (m_scope.isEmpty()) {
        return decl;
    }

    const bool isClassContext = (m_context && m_context->type() == DUContext::Class);
    const QLatin1String scopeType = isClassContext ? QLatin1String("struct ") : QLatin1String("namespace ");
    const QLatin1String scopeClose = isClassContext ? QLatin1String(";") : QLatin1String("");

    QString ret;
    const auto scopes = m_scope.toStringList();
    for (const QString& scope : scopes) {
        ret += scopeType + scope + QLatin1String(" {\n");
    }

    ret += decl;
    ret += QString(QLatin1Char('}') + scopeClose + QLatin1Char('\n')).repeated(m_scope.count());

    return ret;
}

SourceCodeInsertion::SourceCodeInsertion(TopDUContext* topContext)
    : m_context(topContext)
    , m_topContext(topContext)
    , m_codeRepresentation(createCodeRepresentation(m_topContext->url()))
{
}

SourceCodeInsertion::~SourceCodeInsertion()
{
}

KTextEditor::Cursor SourceCodeInsertion::end() const
{
    auto ret = m_context->rangeInCurrentRevision().end();
    if (m_codeRepresentation && m_codeRepresentation->lines() && dynamic_cast<TopDUContext*>(m_context.data())) {
        ret.setLine(m_codeRepresentation->lines() - 1);
        ret.setColumn(m_codeRepresentation->line(ret.line()).size());
    }
    return ret;
}

KTextEditor::Range SourceCodeInsertion::insertionRange(int line)
{
    if (line == 0 || !m_codeRepresentation) {
        return KTextEditor::Range(line, 0, line, 0);
    }

    KTextEditor::Range range(line - 1, m_codeRepresentation->line(line - 1).size(), line - 1,
                             m_codeRepresentation->line(line - 1).size());
    // If the context finishes on that line, then this will need adjusting
    if (!m_context->rangeInCurrentRevision().contains(range)) {
        range.start() = m_context->rangeInCurrentRevision().end();
        if (range.start().column() > 0) {
            range.start() = range.start() - KTextEditor::Cursor(0, 1);
        }
        range.end() = range.start();
    }

    return range;
}

bool SourceCodeInsertion::insertFunctionDeclaration(KDevelop::Declaration* declaration, const Identifier& id, const QString& body)
{
    if (!m_context) {
        return false;
    }

    Signature signature;
    const auto localDeclarations = declaration->internalContext()->localDeclarations();
    signature.parameters.reserve(localDeclarations.count());
    std::transform(localDeclarations.begin(), localDeclarations.end(),
                   std::back_inserter(signature.parameters),
                   [] (Declaration* argument) -> ParameterItem
                   { return {IndexedType(argument->indexedType()), argument->identifier().toString()}; });

    auto funcType = declaration->type<FunctionType>();
    auto returnType = funcType->returnType();
    if (auto classFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(declaration)) {
        if (classFunDecl->isConstructor() || classFunDecl->isDestructor()) {
            returnType = nullptr;
        }
    }
    signature.returnType = IndexedType(returnType);
    signature.isConst = funcType->modifiers() & AbstractType::ConstModifier;

    QString decl = CodegenHelper::makeSignatureString(declaration, signature, true);
    decl.replace(declaration->qualifiedIdentifier().toString(), id.toString());

    if (body.isEmpty()) {
        decl += QLatin1Char(';');
    } else {
        if (!body.startsWith(QLatin1Char(' ')) && !body.startsWith(QLatin1Char('\n'))) {
            decl += QLatin1Char(' ');
        }
        decl += zeroIndentation(body);
    }

    int line = findInsertionPoint();

    decl = QLatin1String("\n\n") + applySubScope(decl);
    const auto formatter = ICore::self()->sourceFormatterController()->fileFormatter(declaration->url().toUrl());
    if (formatter) {
        decl = formatter->format(decl);
    }

    return m_changeSet.addChange(DocumentChange(m_context->url(), insertionRange(line), QString(), decl));
}

int SourceCodeInsertion::findInsertionPoint() const
{
    int line = end().line();

    const auto localDeclarations = m_context->localDeclarations();
    for (auto* decl : localDeclarations) {
        if (m_context->type() == DUContext::Class) {
            continue;
        }

        if (!dynamic_cast<AbstractFunctionDeclaration*>(decl)) {
            continue;
        }

        line = decl->range().end.line + 1;
        if (decl->internalContext()) {
            line = decl->internalContext()->range().end.line + 1;
        }
    }

    clangDebug() << line << m_context->scopeIdentifier(true) << m_context->rangeInCurrentRevision()
             << m_context->url().toUrl() << m_context->parentContext();
    clangDebug() << "count of declarations:" << m_context->topContext()->localDeclarations().size();

    return line;
}
