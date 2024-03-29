/*
    SPDX-FileCopyrightText: 2010-2011 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CONTROLFLOWNODE_H
#define KDEVPLATFORM_CONTROLFLOWNODE_H
#include <language/editor/rangeinrevision.h>
#include <language/languageexport.h>

namespace KDevelop {
/** @brief Class that represents a node in the control flow graph

    This class will provide us the information to know how to navigate in a ControlFlowGraph.
    It has two methods (next and alternative) that will tell us what are the next nodes and
    nodeRange and conditionRange that will tell us the code ranges that this nodes is located at.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ControlFlowNode
{
public:
    /** Defines the type of the node in terms of what's on the next and alternative method */
    enum Type {
        Conditional, /**< It's a conditional node. alternative and next are available, also conditionRange returns a valid range. */
        Sequential,  /**< It's a node where we just have a next node, we always know where it's going to go as a next step. */
        Exit         /**< It's the end node, it will either return to the caller or finish execution depending on the context. */
    };

    /** Constructs an empty node with no next or alternative nodes */
    ControlFlowNode();

    /** @returns the node type by checking the node's next and alternative value */
    Type type() const;

    /** Sets where is this range going to start to @p cursor*/
    void setStartCursor(const CursorInRevision& cursor);

    /** Sets where is this range going to end to @p cursor*/
    void setEndCursor(const CursorInRevision& cursor);

    /** Sets the condition range to @p range */
    void setConditionRange(const KDevelop::RangeInRevision& range);

    /** Sets @p next to be the node that will be executed after this one */
    void setNext(ControlFlowNode* next);

    /** Sets @p alt to be the alternative to next. Converts this node into a conditional node */
    void setAlternative(ControlFlowNode* alt);

    /** @returns the node to be executed next */
    ControlFlowNode* next() const;

    /** @returns the node to be executed next alternatively */
    ControlFlowNode* alternative() const;

    /** @returns the node range as in what range area does the node affect. */
    KDevelop::RangeInRevision nodeRange() const;

    /** @returns the node range as in what range does its condition affect. */
    KDevelop::RangeInRevision conditionRange() const;

private:
    KDevelop::RangeInRevision m_nodeRange;
    KDevelop::RangeInRevision m_conditionRange;

    ControlFlowNode* m_next = nullptr;
    ControlFlowNode* m_alternative = nullptr;
};
}
#endif // KDEVPLATFORM_FLUXNODE_H
