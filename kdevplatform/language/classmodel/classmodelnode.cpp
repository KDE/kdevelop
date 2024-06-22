/*
    SPDX-FileCopyrightText: 2007-2009 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "classmodelnode.h"

#include <typeinfo>
#include <KLocalizedString>

#include "../duchain/duchainlock.h"
#include "../duchain/duchain.h"
#include "../duchain/persistentsymboltable.h"
#include "../duchain/duchainutils.h"
#include "../duchain/classdeclaration.h"
#include "../duchain/classfunctiondeclaration.h"
#include "../duchain/types/functiontype.h"
#include "../duchain/types/enumerationtype.h"

#include <debug.h>

using namespace KDevelop;
using namespace ClassModelNodes;

IdentifierNode::IdentifierNode(KDevelop::Declaration* a_decl,
                               NodesModelInterface* a_model,
                               const QString& a_displayName)
    : DynamicNode(a_displayName.isEmpty() ? a_decl->identifier().toString() : a_displayName, a_model)
    , m_identifier(a_decl->qualifiedIdentifier())
    , m_indexedDeclaration(a_decl)
    , m_cachedDeclaration(a_decl)
{
}

Declaration* IdentifierNode::declaration()
{
    if (!m_cachedDeclaration)
        m_cachedDeclaration = m_indexedDeclaration.declaration();

    return m_cachedDeclaration.data();
}

bool IdentifierNode::getIcon(QIcon& a_resultIcon)
{
    DUChainReadLocker readLock(DUChain::lock());

    Declaration* decl = declaration();
    if (decl)
        a_resultIcon = DUChainUtils::iconForDeclaration(decl);

    return !a_resultIcon.isNull();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

EnumNode::EnumNode(KDevelop::Declaration* a_decl, NodesModelInterface* a_model)
    : IdentifierNode(a_decl, a_model)
{
    // Set display name for anonymous enums
    if (m_displayName.isEmpty())
        m_displayName = QStringLiteral("*Anonymous*");
}

bool EnumNode::getIcon(QIcon& a_resultIcon)
{
    DUChainReadLocker readLock(DUChain::lock());

    auto* decl = dynamic_cast<ClassMemberDeclaration*>(declaration());
    if (decl == nullptr) {
        a_resultIcon = QIcon::fromTheme(QStringLiteral("enum"));
    } else
    {
        if (decl->accessPolicy() == Declaration::Protected) {
            a_resultIcon = QIcon::fromTheme(QStringLiteral("protected_enum"));
        } else if (decl->accessPolicy() == Declaration::Private) {
            a_resultIcon = QIcon::fromTheme(QStringLiteral("private_enum"));
        } else
        {
            a_resultIcon = QIcon::fromTheme(QStringLiteral("enum"));
        }
    }

    return true;
}

void EnumNode::populateNode()
{
    DUChainReadLocker readLock(DUChain::lock());

    Declaration* decl = declaration();

    if (decl->internalContext()) {
        const auto localDeclarations = decl->internalContext()->localDeclarations();
        for (Declaration* enumDecl : localDeclarations) {
            addNode(new EnumNode(enumDecl, m_model));
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ClassNode::ClassNode(Declaration* a_decl, NodesModelInterface* a_model)
    : IdentifierNode(a_decl, a_model)
{
}

ClassNode::~ClassNode()
{
    if (!m_cachedUrl.isEmpty()) {
        ClassModelNodesController::self().unregisterForChanges(m_cachedUrl, this);
        m_cachedUrl = IndexedString();
    }
}

void ClassNode::populateNode()
{
    DUChainReadLocker readLock(DUChain::lock());

    if (m_model->features().testFlag(NodesModelInterface::ClassInternals)) {
        if (updateClassDeclarations()) {
            m_cachedUrl = declaration()->url();
            ClassModelNodesController::self().registerForChanges(m_cachedUrl, this);
        }
    }

    // Add special folders
    if (m_model->features().testFlag(NodesModelInterface::BaseAndDerivedClasses))
        addBaseAndDerived();
}

bool ClassNode::updateClassDeclarations()
{
    bool hadChanges = false;
    SubIdentifiersMap existingIdentifiers = m_subIdentifiers;

    auto* klass = dynamic_cast<ClassDeclaration*>(declaration());

    if (klass) {
        const auto localDeclarations = klass->internalContext()->localDeclarations();
        for (Declaration* decl : localDeclarations) {
            // Ignore forward declarations.
            if (decl->isForwardDeclaration())
                continue;

            // Don't add existing declarations.
            const auto identifierIt = existingIdentifiers.find(decl->ownIndex());
            if (identifierIt != existingIdentifiers.end()) {
                existingIdentifiers.erase(identifierIt);
                continue;
            }

            Node* newNode = nullptr;

            if (EnumerationType::Ptr enumType = decl->type<EnumerationType>())
                newNode = new EnumNode(decl, m_model);
            else if (decl->isFunctionDeclaration())
                newNode = new FunctionNode(decl, m_model);
            else if (auto* classDecl = dynamic_cast<ClassDeclaration*>(decl))
                newNode = new ClassNode(classDecl, m_model);
            else if (auto* memDecl = dynamic_cast<ClassMemberDeclaration*>(decl))
                newNode = new ClassMemberNode(memDecl, m_model);
            else
            {
                // Debug - for reference.
                qCDebug(LANGUAGE) << "class: " << klass->toString() << "name: " << decl->toString() <<
                    " - unknown declaration type: " << typeid(*decl).name();
            }

            if (newNode) {
                addNode(newNode);

                // Also remember the identifier.
                m_subIdentifiers.insert(decl->ownIndex(), newNode);

                hadChanges = true;
            }
        }
    }

    // Remove old existing identifiers
    for (SubIdentifiersMap::iterator iter = existingIdentifiers.begin();
         iter != existingIdentifiers.end();
         ++iter) {
        iter.value()->removeSelf();
        m_subIdentifiers.remove(iter.key());
        hadChanges = true;
    }

    return hadChanges;
}

bool ClassNode::addBaseAndDerived()
{
    bool added = false;

    auto* baseClassesNode = new BaseClassesFolderNode(m_model);
    addNode(baseClassesNode);
    if (!baseClassesNode->hasChildren())
        removeNode(baseClassesNode);
    else
        added = true;

    auto* derivedClassesNode = new DerivedClassesFolderNode(m_model);
    addNode(derivedClassesNode);
    if (!derivedClassesNode->hasChildren())
        removeNode(derivedClassesNode);
    else
        added = true;

    return added;
}

void ClassNode::nodeCleared()
{
    if (!m_cachedUrl.isEmpty()) {
        ClassModelNodesController::self().unregisterForChanges(m_cachedUrl, this);
        m_cachedUrl = IndexedString();
    }

    m_subIdentifiers.clear();
}

void ClassModelNodes::ClassNode::documentChanged(const KDevelop::IndexedString&)
{
    DUChainReadLocker readLock(DUChain::lock());

    if (updateClassDeclarations())
        recursiveSort();
}

ClassNode* ClassNode::findSubClass(const KDevelop::IndexedQualifiedIdentifier& a_id)
{
    // Make sure we have sub nodes.
    performPopulateNode();

    /// @todo This is slow - we go over all the sub identifiers but the assumption is that
    ///       this function call is rare and the list is not that long.
    for (Node* item : std::as_const(m_subIdentifiers)) {
        auto* classNode = dynamic_cast<ClassNode*>(item);
        if (classNode == nullptr)
            continue;

        if (classNode->identifier() == a_id)
            return classNode;
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FunctionNode::FunctionNode(Declaration* a_decl, NodesModelInterface* a_model)
    : IdentifierNode(a_decl, a_model)
{
    // Append the argument signature to the identifier's name (which is what the displayName is.
    if (FunctionType::Ptr type = a_decl->type<FunctionType>())
        m_displayName += type->partToString(FunctionType::SignatureArguments);

    // Add special values for ctor / dtor to sort first
    auto* classmember = dynamic_cast<ClassFunctionDeclaration*>(a_decl);
    if (classmember) {
        if (classmember->isConstructor() || classmember->isDestructor())
            m_sortableString = QLatin1Char('0') + m_displayName;
        else
            m_sortableString = QLatin1Char('1') + m_displayName;
    } else {
        m_sortableString = m_displayName;
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ClassMemberNode::ClassMemberNode(KDevelop::ClassMemberDeclaration* a_decl, NodesModelInterface* a_model)
    : IdentifierNode(a_decl, a_model)
{
}

bool ClassMemberNode::getIcon(QIcon& a_resultIcon)
{
    DUChainReadLocker readLock(DUChain::lock());

    auto* decl = dynamic_cast<ClassMemberDeclaration*>(declaration());
    if (decl == nullptr)
        return false;

    if (decl->isTypeAlias()) {
        a_resultIcon = QIcon::fromTheme(QStringLiteral("typedef"));
    } else if (decl->accessPolicy() == Declaration::Protected) {
        a_resultIcon = QIcon::fromTheme(QStringLiteral("protected_field"));
    } else if (decl->accessPolicy() == Declaration::Private) {
        a_resultIcon = QIcon::fromTheme(QStringLiteral("private_field"));
    } else
    {
        a_resultIcon = QIcon::fromTheme(QStringLiteral("field"));
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

DynamicFolderNode::DynamicFolderNode(const QString& a_displayName, NodesModelInterface* a_model)
    : DynamicNode(a_displayName, a_model)
{
}

bool DynamicFolderNode::getIcon(QIcon& a_resultIcon)
{
    a_resultIcon = QIcon::fromTheme(QStringLiteral("folder"));
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FolderNode::FolderNode(const QString& a_displayName, NodesModelInterface* a_model)
    : Node(a_displayName, a_model)
{
}

bool FolderNode::getIcon(QIcon& a_resultIcon)
{
    a_resultIcon = QIcon::fromTheme(QStringLiteral("folder"));
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BaseClassesFolderNode::BaseClassesFolderNode(NodesModelInterface* a_model)
    : DynamicFolderNode(i18n("Base classes"), a_model)
{
}

void BaseClassesFolderNode::populateNode()
{
    DUChainReadLocker readLock(DUChain::lock());

    auto* klass = dynamic_cast<ClassDeclaration*>(static_cast<ClassNode*>(parent())->declaration());
    if (klass) {
        // I use the imports instead of the baseClasses in the ClassDeclaration because I need
        // to get to the base class identifier which is not directly accessible through the
        // baseClasses function.
        const auto imports = klass->internalContext()->importedParentContexts();
        for (const DUContext::Import& import : imports) {
            DUContext* baseContext = import.context(klass->topContext());
            if (baseContext && baseContext->type() == DUContext::Class) {
                Declaration* baseClassDeclaration = baseContext->owner();
                if (baseClassDeclaration) {
                    // Add the base class.
                    addNode(new ClassNode(baseClassDeclaration, m_model));
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

DerivedClassesFolderNode::DerivedClassesFolderNode(NodesModelInterface* a_model)
    : DynamicFolderNode(i18n("Derived classes"), a_model)
{
}

void DerivedClassesFolderNode::populateNode()
{
    DUChainReadLocker readLock(DUChain::lock());

    auto* klass = dynamic_cast<ClassDeclaration*>(static_cast<ClassNode*>(parent())->declaration());
    if (klass) {
        uint steps = 10000;
        const QList<Declaration*> inheriters = DUChainUtils::inheriters(klass, steps, true);

        for (Declaration* decl : inheriters) {
            addNode(new ClassNode(decl, m_model));
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

Node::Node(const QString& a_displayName, NodesModelInterface* a_model)
    : m_parentNode(nullptr)
    , m_displayName(a_displayName)
    , m_model(a_model)
{
}

Node::~Node()
{
    // Notify the model about the removal of this nodes' children.
    if (!m_children.empty() && m_model) {
        m_model->nodesAboutToBeRemoved(this, 0, m_children.size() - 1);
        clear();
        m_model->nodesRemoved(this);
    }
}

void Node::clear()
{
    qDeleteAll(m_children);
    m_children.clear();
}

void Node::addNode(Node* a_child)
{
/// @note This is disabled for performance reasons - we add them to the bottom and a
///       sort usually follows which causes a layout change to be fired.
//   m_model->nodesAboutToBeAdded(this, m_children.size(), 1);
    a_child->m_parentNode = this;
    m_children.push_back(a_child);
//   m_model->nodesAdded(this);
}

void Node::removeNode(Node* a_child)
{
    int row = a_child->row();
    m_model->nodesAboutToBeRemoved(this, row, row);
    m_children.removeAt(row);
    delete a_child;
    m_model->nodesRemoved(this);
}

// Sort algorithm for the nodes.
struct SortNodesFunctor
{
    bool operator()(Node* a_lhs, Node* a_rhs)
    {
        if (a_lhs->score() == a_rhs->score()) {
            return a_lhs->sortableString() < a_rhs->sortableString();
        } else
            return a_lhs->score() < a_rhs->score();
    }
};

void Node::recursiveSortInternal()
{
    // Sort my nodes.
    std::sort(m_children.begin(), m_children.end(), SortNodesFunctor());

    // Tell each node to sort it self.
    for (Node* node : std::as_const(m_children)) {
        node->recursiveSortInternal();
    }
}

void Node::recursiveSort()
{
    m_model->nodesLayoutAboutToBeChanged(this);

    recursiveSortInternal();

    m_model->nodesLayoutChanged(this);
}

int Node::row()
{
    if (m_parentNode == nullptr)
        return -1;

    return m_parentNode->m_children.indexOf(this);
}

QIcon ClassModelNodes::Node::cachedIcon()
{
    // Load the cached icon if it's null.
    if (m_cachedIcon.isNull()) {
        if (!getIcon(m_cachedIcon))
            m_cachedIcon = QIcon();
    }

    return m_cachedIcon;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

DynamicNode::DynamicNode(const QString& a_displayName, NodesModelInterface* a_model)
    : Node(a_displayName, a_model)
    , m_populated(false)
{
}

void DynamicNode::collapse()
{
    performNodeCleanup();
}

void DynamicNode::expand()
{
    performPopulateNode();
}

void DynamicNode::performNodeCleanup()
{
    if (!m_populated)
        return;

    if (!m_children.empty()) {
        // Notify model for this node.
        m_model->nodesAboutToBeRemoved(this, 0, m_children.size() - 1);

        // Clear sub-nodes.
        clear();

        m_model->nodesRemoved(this);
    }

    // This shouldn't be called from clear since clear is called also from the d-tor
    // and the function is virtual.
    nodeCleared();

    // Mark the fact that we've been collapsed
    m_populated = false;
}

void DynamicNode::performPopulateNode(bool a_forceRepopulate)
{
    if (m_populated) {
        if (a_forceRepopulate)
            performNodeCleanup();
        else
            return;
    }

    populateNode();

    // We're populated.
    m_populated = true;

    // Sort the list.
    recursiveSort();
}

bool DynamicNode::hasChildren() const
{
    // To get a true status, we'll need to populate the node.
    const_cast<DynamicNode*>(this)->performPopulateNode();

    return !m_children.empty();
}
