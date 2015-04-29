/*
 *   KDevelop outline view
 *   Copyright 2010, 2015 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "outlinenode.h"

#include <language/duchain/duchainutils.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/types/alltypes.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/forwarddeclaration.h>

#include "debug_outline.h"

using namespace KDevelop;

OutlineNode::OutlineNode(const QString& text, OutlineNode* parent)
    : m_cachedText(text)
    , m_parent(parent)
{
}

OutlineNode::OutlineNode(Declaration* decl, OutlineNode* parent)
    : m_decl(decl)
    , m_parent(parent)
{
    // qCDebug(PLUGIN_OUTLINE) << "Adding:" << decl->qualifiedIdentifier().toString() << ": " <<typeid(*decl).name();

    // TODO: properly qualified identifier for out of line function definitions
    m_cachedText = decl->identifier().toString();
    m_cachedIcon = DUChainUtils::iconForDeclaration(decl);
    if (NamespaceAliasDeclaration* alias = dynamic_cast<NamespaceAliasDeclaration*>(decl)) {
        //e.g. C++ using namespace statement
        m_cachedText = alias->importIdentifier().toString();
    }
    else if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(decl)) {
        if (member->isFriend()) {
            m_cachedText = "friend " + m_cachedText;
        }
    }
    if (AbstractType::Ptr type = decl->abstractType()) {
        //add the (function return) type at the end (after a colon - like UML)
        //so that the first thing seen is the name of the function/variable
        //and not the (function return) type
        AbstractType::WhichType typeEnum = type->whichType();
        switch (typeEnum) {
        case AbstractType::TypeFunction: {
            FunctionType::Ptr func = type.cast<FunctionType>();
            // func->partToString() does not add the argument names -> do it manually
            if (DUContext* fCtx = DUChainUtils::getFunctionContext(decl)) {
                m_cachedText += '(';
                bool first = true;
                foreach (Declaration* childDecl, fCtx->localDeclarations(decl->topContext())) {
                    if (first) {
                        first = false;
                    } else {
                        m_cachedText += QStringLiteral(", ");
                    }
                    m_cachedText += childDecl->abstractType()->toString();
                    auto ident = childDecl->identifier();
                    if (!ident.isEmpty()) {
                        m_cachedText += ' ' +  ident.toString();
                    }

                }
                m_cachedText += ')';
            } else {
                qCWarning(PLUGIN_OUTLINE) << "Missing function context:" << decl->qualifiedIdentifier().toString();
                m_cachedText += func->partToString(FunctionType::SignatureArguments);
            }
            //constructors/destructors have no return type, a trailing semicolon would look stupid
            if (func->returnType()) {
                m_cachedText += " : " + func->partToString(FunctionType::SignatureReturn);
            }
            return; // don't append any children here!
        }
        case AbstractType::TypeEnumeration:
            //no need to append the fully qualified type
            break;
        case AbstractType::TypeEnumerator:
            //no need to append the fully qualified type
            Q_ASSERT(decl->type<EnumeratorType>());
            m_cachedText += " = " + decl->type<EnumeratorType>()->valueAsString();
            break;
        case AbstractType::TypeStructure: {
            //this seems to be the way it has to be done (after grepping through source code)
            //TODO shouldn't there be some kind of isFriend() functionality?
            static IndexedIdentifier friendIdentifier(Identifier("friend"));
            const bool isFriend = decl->indexedIdentifier() == friendIdentifier;
            if (isFriend) {
                //FIXME There seems to be no way of finding out whether the friend is class/struct/etc
                m_cachedText += ' ' + type->toString();
            }
            break;
        }
        case AbstractType::TypeAlias: {
            //append the type it aliases
            TypeAliasType::Ptr alias = type.cast<TypeAliasType>();
            if (AbstractType::Ptr targetType = alias->type()) {
                m_cachedText += " : " + targetType->toString();
            }
        }
        break;
        default:
            QString typeStr = type->toString();
            if (!typeStr.isEmpty()) {
                m_cachedText += " : " + typeStr;
            }
        }
    }


    //these two don't seem to be hit
    if (decl->isAutoDeclaration()) {
        m_cachedText = "Implicit: " + m_cachedText;
    }
    if (decl->isAnonymous()) {
        m_cachedText = "<anonymous>" + m_cachedText;
    }

    if (DUContext* ctx = decl->internalContext()) {
        appendContext(ctx, decl->topContext());
    }
    if (m_cachedText.isEmpty()) {
        m_cachedText = i18nc("An anonymous declaration (class, function, etc.)", "<anonymous>");
    }
}

void OutlineNode::appendContext(DUContext* ctx, TopDUContext* top)
{
    foreach (Declaration* childDecl, ctx->localDeclarations(top)) {
        if (childDecl) {
            m_children.emplace_back(childDecl, this);
        }
    }
    if (ctx->type() != DUContext::Template) {
        return; //no need to list these subcontexts
    }
    foreach (DUContext* childContext, ctx->childContexts()) {
        if (!childContext)
            continue;
        QVector<Declaration*> decls = childContext->localDeclarations(top);
        if (decls.isEmpty()) {
            continue;
        }
        QString ctxName = childContext->scopeIdentifier(true).toString();
        // if child context is a template context or if name is empty append to current list,
        // otherwise create a new context node
        OutlineNode* childNode = 0;
        if (childContext->type() == DUContext::Template || ctxName.isEmpty()) {
            childNode = this;
        } else {
            m_children.emplace_back(ctxName, this);
        }
        //append all subcontexts recursively
        childNode->appendContext(childContext, top);
    }
}


OutlineNode::~OutlineNode()
{
}
