/*
    SPDX-FileCopyrightText: 2010, 2015 Alex Richardson <alex.richardson@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <QString>
#include <QIcon>
#include <memory>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainbase.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainpointer.h>


namespace KDevelop {
class Declaration;
class DUContext;
}

class OutlineNode
{
    Q_DISABLE_COPY(OutlineNode)
    void appendContext(KDevelop::DUContext* ctx, KDevelop::TopDUContext* top);
    void sortByLocation(bool requiresSorting);
public:
    OutlineNode(const QString& text, OutlineNode* parent);
    OutlineNode(OutlineNode&& other) Q_DECL_NOEXCEPT;
    OutlineNode& operator=(OutlineNode&& other) Q_DECL_NOEXCEPT;
    OutlineNode(KDevelop::Declaration* decl, OutlineNode* parent);
    OutlineNode(KDevelop::DUContext* ctx, const QString& name, OutlineNode* parent);
    virtual ~OutlineNode();
    QIcon icon() const;
    QString text() const;
    const OutlineNode* parent() const;
    const std::vector<OutlineNode>& children() const;
    int childCount() const;
    const OutlineNode* childAt(int index) const;
    int indexOf(const OutlineNode* child) const;
    static std::unique_ptr<OutlineNode> fromTopContext(KDevelop::TopDUContext* ctx);
    static std::unique_ptr<OutlineNode> dummyNode();
    KDevelop::DUChainBase* duChainObject() const;
    friend void swap(OutlineNode& n1, OutlineNode& n2);
private:
    QString m_cachedText;
    QIcon m_cachedIcon;
    KDevelop::DUChainBasePointer m_declOrContext;
    OutlineNode* m_parent;
    std::vector<OutlineNode> m_children;
};

inline int OutlineNode::childCount() const
{
    return static_cast<int>(m_children.size());
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
    const auto max = m_children.size();
    // Comparing the address here is only fine since we never modify the vector after initial creation
    for (size_t i = 0; i < max; i++) {
        if (child == &m_children[i]) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

inline QIcon OutlineNode::icon() const
{
    return m_cachedIcon;
}

inline QString OutlineNode::text() const
{
    return m_cachedText;
}

inline KDevelop::DUChainBase* OutlineNode::duChainObject() const
{
    ENSURE_CHAIN_READ_LOCKED
    return m_declOrContext.data();
}


inline OutlineNode::OutlineNode(OutlineNode&& other) Q_DECL_NOEXCEPT
    : m_cachedText(std::move(other.m_cachedText))
    , m_cachedIcon(std::move(other.m_cachedIcon))
    , m_declOrContext(std::move(other.m_declOrContext))
    , m_parent(std::move(other.m_parent))
    , m_children(std::move(other.m_children))
{
    // qDebug("Move ctor %p -> %p", &other, this);
    other.m_parent = nullptr;
    other.m_declOrContext = nullptr;
    for (OutlineNode& child : m_children) {
        // when we are moved the parent pointer has to be updated for the children!
        child.m_parent = this;
    }
}

inline OutlineNode& OutlineNode::operator=(OutlineNode&& other) Q_DECL_NOEXCEPT
{
    if (this == &other) {
        return *this;
    }
    m_cachedText = std::move(other.m_cachedText);
    m_cachedIcon = std::move(other.m_cachedIcon);
    m_declOrContext = std::move(other.m_declOrContext);
    m_parent = std::move(other.m_parent);
    m_children = std::move(other.m_children);
    // qDebug("Move assignment %p -> %p", &other, this);
    other.m_parent = nullptr;
    other.m_declOrContext = nullptr;
    for (OutlineNode& child : m_children) {
        // when we are moved the parent pointer has to be updated for the children!
        child.m_parent = this;
    }
    return *this;
}

inline void swap(OutlineNode& n1, OutlineNode& n2)
{
    // For some reason std::sort only sometimes calls swap and mostly uses move ctor + assign.
    // Probably it uses different algorithms for different sequence sizes
    // qDebug("Swapping %p and %p", &n1, &n2);
    std::swap(n1.m_cachedText, n2.m_cachedText);
    std::swap(n1.m_cachedIcon, n2.m_cachedIcon);
    std::swap(n1.m_declOrContext, n2.m_declOrContext);
    std::swap(n1.m_parent, n2.m_parent);
    std::swap(n1.m_children, n2.m_children);
}
