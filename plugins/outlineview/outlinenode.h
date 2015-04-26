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

#include <language/duchain/duchainpointer.h>

namespace KDevelop {
class Declaration;
}

class OutlineNode {
    Q_DISABLE_COPY(OutlineNode)
    void appendContext(KDevelop::DUContext* ctx, KDevelop::TopDUContext* top);
public:
    OutlineNode(const QString& text, OutlineNode* parent);
    OutlineNode(KDevelop::Declaration* decl, OutlineNode* parent);
    virtual ~OutlineNode();
    QIcon icon() const;
    QString text() const;
    OutlineNode* parent() const;
    QList<OutlineNode*> children() const;
    int childCount() const;
    OutlineNode* childAt(int index) const;
    int indexOf(OutlineNode* child) const;
    void reload();
    /** DUChain must be read-locked */
    const KDevelop::Declaration* declaration() const;
private:
    QString m_cachedText;
    QIcon m_cachedIcon;
    QString m_dummy;
    KDevelop::DeclarationPointer m_decl;
    OutlineNode* m_parent;
    QList<OutlineNode*> m_children;
};

inline int OutlineNode::childCount() const
{
    return m_children.size();
}

inline QList<OutlineNode*> OutlineNode::children() const
{
    return m_children;
}

inline OutlineNode* OutlineNode::childAt(int index) const
{
    return m_children.at(index);
}

inline OutlineNode* OutlineNode::parent() const
{
    return m_parent;
}

inline int OutlineNode::indexOf(OutlineNode* child) const
{
    return m_children.indexOf(child);
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
    return m_decl.data();
}
