/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "controlflownode.h"

using namespace KDevelop;

ControlFlowNode::ControlFlowNode()
    : m_conditionRange(RangeInRevision::invalid())
{}

ControlFlowNode::Type ControlFlowNode::type() const
{
    Q_ASSERT(!m_alternative || m_next); //If we have alternative, we have next.

    if (m_next && m_alternative)
        return Conditional;
    else if (m_next)
        return Sequential;
    else
        return Exit;
}

void ControlFlowNode::setConditionRange(const RangeInRevision& range)
{
    Q_ASSERT(!range.isValid() || range.end >= range.start);
    m_conditionRange = range;
}

void ControlFlowNode::setStartCursor(const CursorInRevision& cursor)
{
    m_nodeRange.start = cursor;
}

void ControlFlowNode::setEndCursor(const CursorInRevision& cursor)
{
    m_nodeRange.end = cursor;
}

void ControlFlowNode::setNext(ControlFlowNode* next)
{
    m_next = next;
}

void ControlFlowNode::setAlternative(ControlFlowNode* alt)
{
    m_alternative = alt;
}

ControlFlowNode* ControlFlowNode::next() const
{
    return m_next;
}

ControlFlowNode* ControlFlowNode::alternative() const
{
    return m_alternative;
}

RangeInRevision ControlFlowNode::nodeRange() const
{
    return m_nodeRange;
}

RangeInRevision ControlFlowNode::conditionRange() const
{
    return m_conditionRange;
}
