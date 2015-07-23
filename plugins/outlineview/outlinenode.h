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
#pragma once

#include <QString>
#include <QIcon>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainpointer.h>

namespace KDevelop {
class Declaration;
}

class OutlineNode
{
    Q_DISABLE_COPY(OutlineNode)
    void appendContext(KDevelop::DUContext* ctx, KDevelop::TopDUContext* top);
public:
    OutlineNode(const QString& text, OutlineNode* parent);
    explicit OutlineNode(OutlineNode&& other) noexcept;
    OutlineNode(KDevelop::Declaration* decl, OutlineNode* parent);
    virtual ~OutlineNode();
    QIcon icon() const;
    QString text() const;
    const OutlineNode* parent() const;
    const std::vector<OutlineNode>& children() const;
    int childCount() const;
    const OutlineNode* childAt(int index) const;
    int indexOf(const OutlineNode* child) const;
    /** DUChain must be read-locked */
    const KDevelop::Declaration* declaration() const;
    static ssize_t findNode(const std::vector<OutlineNode>& vec, const OutlineNode* n);
private:
    QString m_cachedText;
    QIcon m_cachedIcon;
    KDevelop::DeclarationPointer m_decl;
    OutlineNode* m_parent;
    std::vector<OutlineNode> m_children;
};

inline int OutlineNode::childCount() const
{
    return m_children.size();
}

inline const std::vector<OutlineNode>& OutlineNode::children() const
{
    return m_children;
}

inline const OutlineNode* OutlineNode::childAt(int index) const
{
    return &m_children.at(index);
}

inline const OutlineNode* OutlineNode::parent() const
{
    return m_parent;
}

inline int OutlineNode::indexOf(const OutlineNode* child) const
{
    return findNode(m_children, child);
}

inline QIcon OutlineNode::icon() const
{
    return m_cachedIcon;
}

inline QString OutlineNode::text() const
{
    return m_cachedText;
}

inline const KDevelop::Declaration* OutlineNode::declaration() const
{
    Q_ASSERT(KDevelop::DUChain::lock()->currentThreadHasReadLock());
    return m_decl.data();
}

inline ssize_t OutlineNode::findNode(const std::vector< OutlineNode >& vec, const OutlineNode* n)
{
    const auto max = vec.size();
    for (size_t i = 0; i < max; i++) {
        if (n == &vec[i]) {
            return i;
        }
    }
    return -1;
}

inline OutlineNode::OutlineNode(OutlineNode&& other) noexcept
    : m_parent(other.m_parent)
{
    std::swap(m_children, other.m_children);
    std::swap(m_cachedIcon, other.m_cachedIcon);
    std::swap(m_cachedText, other.m_cachedText);
    std::swap(m_decl, other.m_decl);
    other.m_parent = nullptr;
    for (OutlineNode& child : m_children) {
        // when we are moved the parent pointer has to change as well
        child.m_parent = this;
    }
}
