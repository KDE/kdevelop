/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "normaldeclarationcompletionitem.h"
#include "codecompletionmodel.h"
#include "../duchain/duchainlock.h"
#include "../duchain/duchain.h"
#include "../duchain/classfunctiondeclaration.h"
#include "../duchain/types/functiontype.h"
#include "../duchain/types/enumeratortype.h"
#include "../duchain/duchainutils.h"
#include "../duchain/navigation/abstractdeclarationnavigationcontext.h"
#include <debug.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>

namespace KDevelop {
const int NormalDeclarationCompletionItem::normalBestMatchesCount = 5;
//If this is true, the return-values of argument-hints will be just written as "..." if they are too long
const bool NormalDeclarationCompletionItem::shortenArgumentHintReturnValues = true;
const int NormalDeclarationCompletionItem::maximumArgumentHintReturnValueLength = 30;
const int NormalDeclarationCompletionItem::desiredTypeLength = 20;

NormalDeclarationCompletionItem::NormalDeclarationCompletionItem(const KDevelop::DeclarationPointer& decl,
                                                                 const QExplicitlySharedDataPointer<CodeCompletionContext>& context,
                                                                 int inheritanceDepth)
    : m_completionContext(context)
    , m_declaration(decl)
    , m_inheritanceDepth(inheritanceDepth)
{
}

KDevelop::DeclarationPointer NormalDeclarationCompletionItem::declaration() const
{
    return m_declaration;
}

QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext> NormalDeclarationCompletionItem::completionContext() const
{
    return m_completionContext;
}

int NormalDeclarationCompletionItem::inheritanceDepth() const
{
    return m_inheritanceDepth;
}

int NormalDeclarationCompletionItem::argumentHintDepth() const
{
    if (m_completionContext)
        return m_completionContext->depth();
    else
        return 0;
}

QString NormalDeclarationCompletionItem::declarationName() const
{
    if (!m_declaration) {
        return QStringLiteral("<unknown>");
    }
    QString ret = m_declaration->identifier().toString();
    if (ret.isEmpty())
        return QStringLiteral("<unknown>");
    else
        return ret;
}

void NormalDeclarationCompletionItem::execute(KTextEditor::View* view, const KTextEditor::Range& word)
{
    if (m_completionContext && m_completionContext->depth() != 0)
        return; //Do not replace any text when it is an argument-hint

    KTextEditor::Document* document = view->document();
    QString newText;

    {
        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        if (m_declaration) {
            newText = declarationName();
        } else {
            qCDebug(LANGUAGE) << "Declaration disappeared";
            return;
        }
    }

    document->replaceText(word, newText);
    KTextEditor::Range newRange = word;
    newRange.setEnd(KTextEditor::Cursor(newRange.end().line(), newRange.start().column() + newText.length()));

    executed(view, newRange);
}

QWidget* NormalDeclarationCompletionItem::createExpandingWidget(const KDevelop::CodeCompletionModel* model) const
{
    Q_UNUSED(model);
    return nullptr;
}

bool NormalDeclarationCompletionItem::createsExpandingWidget() const
{
    return false;
}

QString NormalDeclarationCompletionItem::shortenedTypeString(const KDevelop::DeclarationPointer& decl,
                                                             int desiredTypeLength) const
{
    Q_UNUSED(desiredTypeLength);
    return decl->abstractType()->toString();
}

void NormalDeclarationCompletionItem::executed(KTextEditor::View* view, const KTextEditor::Range& word)
{
    Q_UNUSED(view);
    Q_UNUSED(word);
}

QVariant NormalDeclarationCompletionItem::data(const QModelIndex& index, int role,
                                               const KDevelop::CodeCompletionModel* model) const
{
    DUChainReadLocker lock(DUChain::lock(), 500);
    if (!lock.locked()) {
        qCDebug(LANGUAGE) << "Failed to lock the du-chain in time";
        return QVariant();
    }

    if (!m_declaration)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == CodeCompletionModel::Name) {
            return declarationName();
        } else if (index.column() == CodeCompletionModel::Postfix) {
            if (FunctionType::Ptr functionType = m_declaration->type<FunctionType>()) {
                // Retrieve const/volatile string
                return functionType->AbstractType::toString();
            }
        } else if (index.column() == CodeCompletionModel::Prefix) {
            if (m_declaration->kind() == Declaration::Namespace)
                return QStringLiteral("namespace");
            if (m_declaration->abstractType()) {
                if (EnumeratorType::Ptr enumerator = m_declaration->type<EnumeratorType>()) {
                    if (m_declaration->context()->owner() && m_declaration->context()->owner()->abstractType()) {
                        if (!m_declaration->context()->owner()->identifier().isEmpty())
                            return shortenedTypeString(DeclarationPointer(
                                                           m_declaration->context()->owner()), desiredTypeLength);
                        else
                            return QStringLiteral("enum");
                    }
                }
                if (FunctionType::Ptr functionType = m_declaration->type<FunctionType>()) {
                    auto* funDecl = dynamic_cast<ClassFunctionDeclaration*>(m_declaration.data());

                    if (functionType->returnType()) {
                        QString ret = shortenedTypeString(m_declaration, desiredTypeLength);
                        if (shortenArgumentHintReturnValues && argumentHintDepth() &&
                            ret.length() > maximumArgumentHintReturnValueLength)
                            return QStringLiteral("...");
                        else
                            return ret;
                    } else if (argumentHintDepth()) {
                        return QString();//Don't show useless prefixes in the argument-hints
                    } else if (funDecl && funDecl->isConstructor())
                        return QStringLiteral("<constructor>");
                    else if (funDecl && funDecl->isDestructor())
                        return QStringLiteral("<destructor>");
                    else
                        return QStringLiteral("<incomplete type>");
                } else {
                    return shortenedTypeString(m_declaration, desiredTypeLength);
                }
            } else {
                return QStringLiteral("<incomplete type>");
            }
        } else if (index.column() == CodeCompletionModel::Arguments) {
            if (m_declaration->isFunctionDeclaration()) {
                auto functionType = declaration()->type<FunctionType>();
                return functionType ? functionType->partToString(FunctionType::SignatureArguments) : QVariant();
            }
        }
        break;
    case CodeCompletionModel::BestMatchesCount:
        return QVariant(normalBestMatchesCount);
    case CodeCompletionModel::IsExpandable:
        return QVariant(createsExpandingWidget());
    case CodeCompletionModel::ExpandingWidget: {
        if (!createsExpandingWidget())
            return {};

        QWidget* nav = createExpandingWidget(model);
        Q_ASSERT(nav);
        return QVariant::fromValue<QWidget *>(nav);
    }
    case CodeCompletionModel::ScopeIndex:
        return static_cast<int>(reinterpret_cast<quintptr>(m_declaration->context()));

    case CodeCompletionModel::CompletionRole:
        return ( int )completionProperties();
    case CodeCompletionModel::ItemSelected: {
        NavigationContextPointer ctx(new AbstractDeclarationNavigationContext(DeclarationPointer(
                                                                                  m_declaration),
                                                                              TopDUContextPointer()));
        return ctx->html(true);
    }
    case Qt::DecorationRole:
    {
        if (index.column() == CodeCompletionModel::Icon) {
            CodeCompletionModel::CompletionProperties p = completionProperties();
            lock.unlock();
            return DUChainUtils::iconForProperties(p);
        }
        break;
    }
    }
    return QVariant();
}
}
