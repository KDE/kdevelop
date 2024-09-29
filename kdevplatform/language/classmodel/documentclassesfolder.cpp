/*
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentclassesfolder.h"

#include "../duchain/declaration.h"
#include "../duchain/duchainlock.h"
#include "../duchain/duchain.h"
#include "../duchain/persistentsymboltable.h"
#include "../duchain/codemodel.h"

#include <QIcon>
#include <QTimer>

#include <boost/foreach.hpp>

using namespace KDevelop;
using namespace ClassModelNodes;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Contains a static list of classes within the namespace.
class ClassModelNodes::StaticNamespaceFolderNode
    : public Node
{
public:
    StaticNamespaceFolderNode(const KDevelop::QualifiedIdentifier& a_identifier, NodesModelInterface* a_model);

    /// Returns the qualified identifier for this node
    const KDevelop::QualifiedIdentifier& qualifiedIdentifier() const { return m_identifier; }

public: // Node overrides
    bool getIcon(QIcon& a_resultIcon) override;
    int score() const override { return 101; }

private:
    /// The namespace identifier.
    KDevelop::QualifiedIdentifier m_identifier;
};

StaticNamespaceFolderNode::StaticNamespaceFolderNode(const KDevelop::QualifiedIdentifier& a_identifier,
                                                     NodesModelInterface* a_model)
    : Node(a_identifier.last().toString(), a_model)
    , m_identifier(a_identifier)
{
}

bool StaticNamespaceFolderNode::getIcon(QIcon& a_resultIcon)
{
    a_resultIcon = QIcon::fromTheme(QStringLiteral("namespace"));
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

DocumentClassesFolder::OpenedFileClassItem::OpenedFileClassItem(const KDevelop::IndexedString& a_file,
                                                                const KDevelop::IndexedQualifiedIdentifier& a_classIdentifier,
                                                                ClassModelNodes::ClassNode* a_nodeItem)
    : file(a_file)
    , classIdentifier(a_classIdentifier)
    , nodeItem(a_nodeItem)
{
}

DocumentClassesFolder::DocumentClassesFolder(const QString& a_displayName, NodesModelInterface* a_model)
    : DynamicFolderNode(a_displayName, a_model)
    , m_updateTimer(new QTimer(this))
{
    // this is the required delay.
    m_updateTimer->setInterval(2000);
    connect(m_updateTimer, &QTimer::timeout, this, &DocumentClassesFolder::updateChangedFiles);
}

void DocumentClassesFolder::updateChangedFiles()
{
    bool hadChanges = false;

    // re-parse changed documents.
    // TODO: m_updatedFiles seems no longer set, check again and remove if
    for (const IndexedString& file : std::as_const(m_updatedFiles)) {
        // Make sure it's one of the monitored files.
        if (m_openFiles.contains(file))
            hadChanges |= updateDocument(file);
    }

    // Processed all files.
    m_updatedFiles.clear();

    // Sort if had changes.
    if (hadChanges)
        recursiveSort();
}

void DocumentClassesFolder::nodeCleared()
{
    // Clear cached namespaces list (node was cleared).
    m_namespaces.clear();

    // Clear open files and classes list
    m_openFiles.clear();
    m_openFilesClasses.clear();

    // Stop the update timer.
    m_updateTimer->stop();
}

void DocumentClassesFolder::populateNode()
{
    // Start updates timer
    m_updateTimer->start();
}

ClassNode* DocumentClassesFolder::findClassNode(const IndexedQualifiedIdentifier& a_id)
{
    // Make sure that the classes node is populated, otherwise
    // the lookup will not work.
    performPopulateNode();

    ClassIdentifierIterator iter = m_openFilesClasses.get<ClassIdentifierIndex>().find(a_id);
    if (iter == m_openFilesClasses.get<ClassIdentifierIndex>().end())
        return nullptr;

    // If the node is invisible - make it visible by going over the identifiers list.
    if (iter->nodeItem == nullptr) {
        QualifiedIdentifier qualifiedIdentifier = a_id.identifier();

        // Ignore zero length identifiers.
        if (qualifiedIdentifier.count() == 0)
            return nullptr;

        ClassNode* closestNode = nullptr;
        int closestNodeIdLen = qualifiedIdentifier.count();

        // First find the closest visible class node by reverse iteration over the id list.
        while ((closestNodeIdLen > 0) && (closestNode == nullptr)) {
            // Omit one from the end.
            --closestNodeIdLen;

            // Find the closest class.
            closestNode = findClassNode(qualifiedIdentifier.mid(0, closestNodeIdLen));
        }

        if (closestNode != nullptr) {
            // Start iterating forward from this node by exposing each class.
            // By the end of this loop, closestNode should hold the actual node.
            while (closestNode && (closestNodeIdLen < qualifiedIdentifier.count())) {
                // Try the next Id.
                ++closestNodeIdLen;
                closestNode = closestNode->findSubClass(qualifiedIdentifier.mid(0, closestNodeIdLen));
            }
        }

        return closestNode;
    }

    return iter->nodeItem;
}

void DocumentClassesFolder::closeDocument(const IndexedString& a_file)
{
    // Get list of nodes associated with this file and remove them.
    std::pair<FileIterator, FileIterator> range = m_openFilesClasses.get<FileIndex>().equal_range(a_file);
    if (range.first != m_openFilesClasses.get<FileIndex>().end()) {
        BOOST_FOREACH(const OpenedFileClassItem &item, range)
        {
            if (item.nodeItem)
                removeClassNode(item.nodeItem);
        }

        // Clear the lists
        m_openFilesClasses.get<FileIndex>().erase(range.first, range.second);
    }

    // Clear the file from the list of monitored documents.
    m_openFiles.remove(a_file);
}

bool DocumentClassesFolder::updateDocument(const KDevelop::IndexedString& a_file)
{
    uint codeModelItemCount = 0;
    const CodeModelItem* codeModelItems;
    CodeModel::self().items(a_file, codeModelItemCount, codeModelItems);

    // List of declared namespaces in this file.
    QSet<QualifiedIdentifier> declaredNamespaces;

    // List of removed classes - it initially contains all the known classes, we'll eliminate them
    // one by one later on when we encounter them in the document.
    QMap<IndexedQualifiedIdentifier, FileIterator> removedClasses;
    {
        std::pair<FileIterator, FileIterator> range = m_openFilesClasses.get<FileIndex>().equal_range(a_file);
        for (FileIterator iter = range.first;
             iter != range.second;
             ++iter) {
            removedClasses.insert(iter->classIdentifier, iter);
        }
    }

    bool documentChanged = false;

    for (uint codeModelItemIndex = 0; codeModelItemIndex < codeModelItemCount; ++codeModelItemIndex) {
        const CodeModelItem& item = codeModelItems[codeModelItemIndex];

        // Don't insert unknown or forward declarations into the class browser
        if (item.kind == CodeModelItem::Unknown || (item.kind & CodeModelItem::ForwardDeclaration))
            continue;

        KDevelop::QualifiedIdentifier id = item.id.identifier();

        // Don't add empty identifiers.
        if (id.count() == 0)
            continue;

        // If it's a namespace, create it in the list.
        if (item.kind & CodeModelItem::Namespace) {
            // This should create the namespace folder and add it to the cache.
            namespaceFolder(id);

            // Add to the locally created namespaces.
            declaredNamespaces.insert(id);
        } else if (item.kind & CodeModelItem::Class) {
            // Ignore empty unnamed classes.
            if (id.last().toString().isEmpty())
                continue;

            // See if it matches our filter?
            if (isClassFiltered(id))
                continue;

            // Is this a new class or an existing class?
            const auto classIt = removedClasses.find(id);
            if (classIt != removedClasses.end()) {
                // It already exist - remove it from the known classes and continue.
                removedClasses.erase(classIt);
                continue;
            }

            // Where should we put this class?
            Node* parentNode = nullptr;

            // Check if it's namespaced and add it to the proper namespace.
            if (id.count() > 1) {
                QualifiedIdentifier parentIdentifier(id.left(-1));

                // Look up the namespace in the cache.
                // If we fail to find it we assume that the parent context is a class
                // and in that case, when the parent class gets expanded, it will show it.
                NamespacesMap::iterator iter = m_namespaces.find(parentIdentifier);
                if (iter != m_namespaces.end()) {
                    // Add to the namespace node.
                    parentNode = iter.value();
                } else
                {
                    // Reaching here means we didn't encounter any namespace declaration in the document
                    // But a class might still be declared under a namespace.
                    // So we'll perform a more through search to see if it's under a namespace.

                    DUChainReadLocker readLock(DUChain::lock());

                    PersistentSymbolTable::self().visitDeclarations(
                        parentIdentifier, [&](const IndexedDeclaration& indexedDeclaration) {
                            // Look for the first valid declaration.
                            if (auto declaration = indexedDeclaration.declaration()) {
                                // See if it should be namespaced.
                                if (declaration->kind() == Declaration::Namespace) {
                                    // This should create the namespace folder and add it to the cache.
                                    parentNode = namespaceFolder(parentIdentifier);

                                    // Add to the locally created namespaces.
                                    declaredNamespaces.insert(parentIdentifier);
                                }

                                return PersistentSymbolTable::VisitorState::Break;
                            }
                            return PersistentSymbolTable::VisitorState::Continue;
                        });
                }
            } else
            {
                // Add to the main root.
                parentNode = this;
            }

            ClassNode* newNode = nullptr;
            if (parentNode != nullptr) {
                // Create the new node and add it.
                IndexedDeclaration decl;
                DUChainReadLocker lock;
                PersistentSymbolTable::self().visitDeclarations(
                    item.id, [&](const IndexedDeclaration& indexedDeclaration) {
                        if (indexedDeclaration.indexedTopContext().url() == a_file) {
                            decl = indexedDeclaration;
                            return PersistentSymbolTable::VisitorState::Break;
                        }
                        return PersistentSymbolTable::VisitorState::Continue;
                    });

                if (decl.isValid()) {
                    newNode = new ClassNode(decl.declaration(), m_model);
                    parentNode->addNode(newNode);
                }
            }

            // Insert it to the map - newNode can be 0 - meaning the class is hidden.
            m_openFilesClasses.insert(OpenedFileClassItem(a_file, id, newNode));
            documentChanged = true;
        }
    }

    // Remove empty namespaces from the list.
    // We need this because when a file gets unloaded, we unload the declared classes in it
    // and if a namespace has no class in it, it'll forever exist and no one will remove it
    // from the children list.
    for (const QualifiedIdentifier& id : std::as_const(declaredNamespaces)) {
        removeEmptyNamespace(id);
    }

    // Clear erased classes.
    for (const FileIterator item : std::as_const(removedClasses)) {
        if (item->nodeItem)
            removeClassNode(item->nodeItem);
        m_openFilesClasses.get<FileIndex>().erase(item);
        documentChanged = true;
    }

    return documentChanged;
}

void DocumentClassesFolder::parseDocument(const IndexedString& a_file)
{
    // Add the document to the list of open files - this means we monitor it.
    m_openFiles.insert(a_file);

    updateDocument(a_file);
}

void DocumentClassesFolder::removeClassNode(ClassModelNodes::ClassNode* a_node)
{
    // Get the parent namespace identifier.
    QualifiedIdentifier parentNamespaceIdentifier;
    if (auto namespaceParent = dynamic_cast<StaticNamespaceFolderNode*>(a_node->parent())) {
        parentNamespaceIdentifier = namespaceParent->qualifiedIdentifier();
    }

    // Remove the node.
    a_node->removeSelf();

    // Remove empty namespace
    removeEmptyNamespace(parentNamespaceIdentifier);
}

void DocumentClassesFolder::removeEmptyNamespace(const QualifiedIdentifier& a_identifier)
{
    // Stop condition.
    if (a_identifier.count() == 0)
        return;

    // Look it up in the cache.
    NamespacesMap::iterator iter = m_namespaces.find(a_identifier);
    if (iter != m_namespaces.end()) {
        if (!(*iter)->hasChildren()) {
            // Remove this node and try to remove the parent node.
            QualifiedIdentifier parentIdentifier = (*iter)->qualifiedIdentifier().left(-1);
            (*iter)->removeSelf();
            m_namespaces.remove(a_identifier);
            removeEmptyNamespace(parentIdentifier);
        }
    }
}

StaticNamespaceFolderNode* DocumentClassesFolder::namespaceFolder(const KDevelop::QualifiedIdentifier& a_identifier)
{
    // Stop condition.
    if (a_identifier.count() == 0)
        return nullptr;

    // Look it up in the cache.
    NamespacesMap::iterator iter = m_namespaces.find(a_identifier);
    if (iter == m_namespaces.end()) {
        // It's not in the cache - create folders up to it.
        Node* parentNode = namespaceFolder(a_identifier.left(-1));
        if (parentNode == nullptr)
            parentNode = this;

        // Create the new node.
        auto* newNode =
            new StaticNamespaceFolderNode(a_identifier, m_model);
        parentNode->addNode(newNode);

        // Add it to the cache.
        m_namespaces.insert(a_identifier, newNode);

        // Return the result.
        return newNode;
    } else
        return *iter;
}

#include "moc_documentclassesfolder.cpp"
