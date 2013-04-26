/*
   Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_DUCHAINCHANGESET_H
#define KDEVPLATFORM_DUCHAINCHANGESET_H

#include <QtCore/QVariant>

#include "../duchain/identifier.h"
#include "../duchain/topducontext.h"
#include "../duchain/declaration.h"

namespace KDevelop {

class DUChainChangeSet;
class DUChainChange;
class DUChainBase;
class DUContextRef;

template <typename AstNode>
class AstNodeRef;

/**
 * \short A reference to an existing read-only DUChain object.
 *
 * This class represents a duchain object (eg, a KDevelop::DUContext),
 * and allows changes to be planned for that object.
 *
 * \todo Evaluate usefulness of changing child contexts - needed?
 *
 * \warning you must not create cyclic references.
 * \author Hamish Rodda <rodda@kde.org>
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainRef
{
    friend class DUChainChangeSet;

public:
    /*virtual ~DUChainRef();

    virtual const DUChainBase* object() const;
    virtual const DUContext* context() const;
    virtual const Declaration* declaration() const;

    virtual DUChainRef* objectRef() const;

    virtual DUChainBase* newObject() const;
    virtual DUContext* newContext() const;
    virtual Declaration* newDeclaration() const;

    const QList<DUChainChange*>& changes() const;

    /// Rename this object, if applicable
    void renameObject(const QualifiedIdentifier& newIdentifier);
    /// Change the access policy
    void setAccessPolicy(Declaration::AccessPolicy newPolicy);

    void deleteChildContext(DUContext* child);
    void insertChildContext(DUContextRef* newChild);

    void deleteDeclaration(Declaration* declaration);
    void insertDeclaration(Declaration* declaration, DUChainBase* afterObject);
    void appendDeclaration(Declaration* declaration);

    AbstractType::Ptr currentType() const;
    void changeType(AbstractType::Ptr newType);
*/
    /**
     * Rewrite the AST which created this duchain object. Eg:
     * - for declarations, the entire declaration.
     * - for contexts, the contents of the context.
     * - for types, the type declaration.
     *
     * \returns a reference to the AST which represents this object as it currently
     *          exists (after any existing duchain changes are applied).  Changes
     *          made to the AST will be applied along with the duchain change set.
     */
/*    template <typename AstNode>
    AstNodeRef<AstNode> * rewriteAst();

    /// Removes a change from this object reference, and deletes it.
    void deleteChange(DUChainChange* change);

protected:
    /// Constructor.  Either takes an existing \a object (\a newObject = false), or a newly created \a object (\a newObject = true)
    DUChainRef(DUChainChangeSet* set, DUChainBase* object, bool newObject);
    /// Constructor.  Takes another object reference.
    DUChainRef(DUChainChangeSet* set, DUChainRef* original);

    /// Adds a change to this object reference. Takes ownership of the \a change.
    DUChainChange* addChange(DUChainChange* change);

private:
    DUChainChangeSet* m_changeSet;
    DUChainBase* m_object;
    DUChainRef* m_objectRef;
    bool m_newObject;

    QList<DUChainChange*> m_changes;*/
};

typedef QList<DUChainRef*> DUChainBaseList;

/**
 * \short Container class for a change to a duchain object.
 *
 * \author Hamish Rodda <rodda@kde.org>
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainChange
{
public:
    enum ChangeTypes {
        Rename,
        ListInsert,
        ListRemove,
        ListClear,
        ItemReplace,
        ItemMove,
        TypeChange
    } type;

    DUChainChange(ChangeTypes t) : type(t) {}

    enum ItemToChange {
        ContextChildren,
        ContextDeclarations
    } itemToChange;

    /// New local identifier (eg. for contexts, the new DUContext::localScopeIdentifier() )
    QualifiedIdentifier newIdentifier;

    /// The new object to occupy this position, if relevant
    DUChainRef* newObject;
    /// The list of objects to occupy this position, if relevant
    DUChainBaseList newList;
    /// The position to apply the object(s) in the list, if relevant
    int listOffset;
    /// The value of the position, if relevant
    QVariant newValue;

    AbstractType::Ptr newType;
};

/**
 * \short A set of changes to a DUChain.
 *
 * This class holds a set of all changes to a DU Chain, and provides an interface
 * to convenience functions provided by the specific language support involved.
 *
 * \author Hamish Rodda <rodda@kde.org>
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainChangeSet
{
public:
    /**
     * Constructor.
     *
     * \param topContext the top context of the read-only DUChain to modify, or set to null if creating
     *                   a new DUChain from scratch.
     */
    DUChainChangeSet(ReferencedTopDUContext topContext);

    /**
     * Destructor, deletes all objects, references and changes owned by this change set.
     */
    virtual ~DUChainChangeSet();

    /**
     * Create a new declaration to be managed by this change set.
     *
     * \returns the new declaration reference
     */
    virtual DUChainRef* newDeclaration() = 0;

    /**
     * Create a new class to be managed by this change set.
     *
     * \returns the new declaration reference
     */
    virtual DUChainRef* newClass() = 0;

    /**
     * Create a new function to be managed by this change set.
     *
     * \returns the new declaration reference
     */
    virtual DUChainRef* newFunction() = 0;

    /**
     * Copy an existing object from a change set.
     *
     * This change set takes ownership, so that
     * the new object will be deleted when the change set is no longer needed.
     *
     * \returns the new object reference
     */
    DUChainRef* copyRef(DUChainRef* ref);
    
    /**
     * Merge another changeset with this one. This changeset
     * takes ownership of all the objects in the other changeset.
     * After the merge, the merged object becomes empty.
     *
     * Both changesets must reference the same TopDuContext.
     */
    DUChainChangeSet & operator<<(DUChainChangeSet & rhs);

    /**
    * Produce a reference to an existing object in this chain, and replace the
    * object with the reference so that modifications to the reference are already
    * integrated into the change set.
    *
    * You may then modify this reference, and the modifications will be applied
    * to the chain when the change set is finalised.
    *
    * \returns a reference to \a source, which you may modify directly.
    */
    DUChainRef* modifyObject(DUChainBase* source);

    /**
    * Copy an existing object (whether from the DUChain or from the change set).
    * Does not insert the object into the chain.
    *
    * You may then modify this reference, and the modifications will be applied to the object when the change set is finalised.
    *
    * \returns a copy of \a source, which you may modify directly.
    */
    DUChainRef* copyObject(DUChainBase* source);

    /**
     * Retrieve the list of object references and changes.
     */
    QList<DUChainRef*> objectRefs() const;
    
    const ReferencedTopDUContext & topDuContext() const;

private:
    ReferencedTopDUContext m_topContext;

    QList<DUChainRef*> m_objectRefs;
};

}

#endif // KDEVPLATFORM_DUCHAINCHANGESET_H
