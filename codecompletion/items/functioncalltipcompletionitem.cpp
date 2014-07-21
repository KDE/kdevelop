/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "functioncalltipcompletionitem.h"

#include <duchain/helper.h>
#include <duchain/functiontype.h>

#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/ducontext.h>

#include <QtGui/QIcon>

using namespace KDevelop;
using namespace QmlJS;

FunctionCalltipCompletionItem::FunctionCalltipCompletionItem(const DeclarationPointer& decl,
                                                             int depth,
                                                             int argumentIndex)
: m_declaration(decl),
  m_depth(depth)
{
    // Ensure that decl has a function type
    if (!decl) {
        return;
    }

    QmlJS::FunctionType::Ptr func = QmlJS::FunctionType::Ptr::dynamicCast(decl->abstractType());

    if (!func) {
        return;
    }

    // Arguments can be fetch from the function declaration (if available), or
    // from its function type
    Declaration* funcDecl = func->declaration(decl->topContext());
    DUContext* internalFunctionContext =
        getInternalFunctionContext(funcDecl ? DeclarationPointer(funcDecl) : decl);
    QStringList arguments;

    if (internalFunctionContext) {
        auto args = internalFunctionContext->allDeclarations(CursorInRevision::invalid(), decl->topContext(), false);

        for (auto pair : args) {
            arguments.append(pair.first->toString());
        }

        if (argumentIndex < args.count()) {
            m_currentArgumentType = args.at(argumentIndex).first->abstractType();
        }
    } else {
        for (auto type : func->arguments()) {
            arguments.append(type->toString());
        }

        if (argumentIndex < func->arguments().count()) {
            m_currentArgumentType = func->arguments().at(argumentIndex);
        }
    }

    // [type] functionName
    if (func->returnType()) {
        m_prefix = func->returnType()->toString() + QLatin1String(" ");
    }

    m_prefix += decl->qualifiedIdentifier().toString();

    // (arg1, arg2, [currentArgument in m_currentArgument], arg4, arg5)
    m_arguments = QLatin1String("(");

    for (int i=0; i<arguments.count(); ++i) {
        if (i > 0) {
            m_arguments += QLatin1String(", ");
        }

        if (i == argumentIndex) {
            m_currentArgumentStart = m_arguments.length();
            m_currentArgumentLength = arguments.at(i).length();
        }

        m_arguments += arguments.at(i);
    }

    m_arguments += QLatin1String(")");
}

AbstractType::Ptr FunctionCalltipCompletionItem::currentArgumentType() const
{
    return m_currentArgumentType;
}

QVariant FunctionCalltipCompletionItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const
{
    Q_UNUSED(model)

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
            case CodeCompletionModel::Prefix:
                return m_prefix;
            case CodeCompletionModel::Arguments:
                return m_arguments;
        }
        break;

    case CodeCompletionModel::ArgumentHintDepth:
        return argumentHintDepth();

    case CodeCompletionModel::CompletionRole:
        return (int)completionProperties();

    case CodeCompletionModel::HighlightingMethod:
        if (index.column() == CodeCompletionModel::Arguments) {
            return (int)CodeCompletionModel::CustomHighlighting;
        }
        break;

    case CodeCompletionModel::CustomHighlight:
        if (index.column() == CodeCompletionModel::Arguments) {
            QTextFormat format;

            format.setBackground(QBrush(QColor::fromRgb(142, 186, 255)));   // Same color as kdev-python
            format.setProperty(QTextFormat::FontWeight, 99);

            return QVariantList()
                << m_currentArgumentStart
                << m_currentArgumentLength
                << format;
        }
        break;

    case Qt::DecorationRole:
        if (index.column() == CodeCompletionModel::Prefix) {
            return DUChainUtils::iconForProperties(completionProperties());
        }
        break;
    }

    return QVariant();
}

DeclarationPointer FunctionCalltipCompletionItem::declaration() const
{
    return m_declaration;
}

int FunctionCalltipCompletionItem::argumentHintDepth() const
{
    return m_depth;
}

int FunctionCalltipCompletionItem::inheritanceDepth() const
{
    return 0;
}

CodeCompletionModel::CompletionProperties FunctionCalltipCompletionItem::completionProperties() const
{
    return CodeCompletionModel::Function;
}
