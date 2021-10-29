/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "codecompletioncontext.h"

#include <debug.h>

#include <util/pushvalue.h>
#include <language/duchain/ducontext.h>

using namespace KDevelop;

using IntPusher = PushValue<int>;

///Extracts the last line from the given string
QString CodeCompletionContext::extractLastLine(const QString& str)
{
    int prevLineEnd = str.lastIndexOf(QLatin1Char('\n'));
    if (prevLineEnd != -1)
        return str.mid(prevLineEnd + 1);
    else
        return str;
}

int completionRecursionDepth = 0;

CodeCompletionContext::CodeCompletionContext(const DUContextPointer& context, const QString& text,
                                             const KDevelop::CursorInRevision& position, int depth)
    : m_text(text)
    , m_depth(depth)
    , m_valid(true)
    , m_position(position)
    , m_duContext(context)
    , m_parentContext(nullptr)
{
    IntPusher(completionRecursionDepth, completionRecursionDepth + 1);

    if (depth > 10) {
        qCWarning(LANGUAGE) << "too much recursion";
        m_valid = false;
        return;
    }

    if (completionRecursionDepth > 10) {
        qCWarning(LANGUAGE) << "too much recursion";
        m_valid = false;
        return;
    }
}

CodeCompletionContext::~CodeCompletionContext()
{
}

int CodeCompletionContext::depth() const
{
    return m_depth;
}

bool CodeCompletionContext::isValid() const
{
    return m_valid;
}

void KDevelop::CodeCompletionContext::setParentContext(
    QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext> newParent)
{
    m_parentContext = newParent;
    int newDepth = m_depth + 1;
    while (newParent) {
        newParent->m_depth = newDepth;
        ++newDepth;
        newParent = newParent->m_parentContext;
    }
}

CodeCompletionContext* CodeCompletionContext::parentContext()
{
    return m_parentContext.data();
}

QList<QExplicitlySharedDataPointer<KDevelop::CompletionTreeElement>> KDevelop::CodeCompletionContext::ungroupedElements()
{
    return QList<QExplicitlySharedDataPointer<KDevelop::CompletionTreeElement>>();
}

KDevelop::DUContext* KDevelop::CodeCompletionContext::duContext() const
{
    return m_duContext.data();
}
