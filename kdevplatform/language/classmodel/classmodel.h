/*
    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CLASSMODEL_H
#define KDEVPLATFORM_CLASSMODEL_H

#include <QAbstractItemModel>
#include "classmodelnode.h"

#include <language/languageexport.h>

class ClassBrowserPlugin;

namespace KDevelop {
class TopDUContext;
class IDocument;
class DUContext;
class IProject;
class DUChainBase;
class IndexedQualifiedIdentifier;
}

namespace ClassModelNodes {
class Node;
class FilteredAllClassesFolder;
class FilteredProjectFolder;
class FolderNode;
class IdentifierNode;
}

/// The model interface accessible from the nodes.
class NodesModelInterface
{
public:
    virtual ~NodesModelInterface();

public:
    enum Feature {
        AllProjectsClasses = 0x1,
        BaseAndDerivedClasses = 0x2,
        ClassInternals = 0x4
    };
    Q_DECLARE_FLAGS(Features, Feature)

    virtual void nodesLayoutAboutToBeChanged(ClassModelNodes::Node* a_parent) = 0;
    virtual void nodesLayoutChanged(ClassModelNodes::Node* a_parent) = 0;
    virtual void nodesAboutToBeRemoved(ClassModelNodes::Node* a_parent, int a_first, int a_last) = 0;
    virtual void nodesRemoved(ClassModelNodes::Node* a_parent) = 0;
    virtual void nodesAboutToBeAdded(ClassModelNodes::Node* a_parent, int a_pos, int a_size) = 0;
    virtual void nodesAdded(ClassModelNodes::Node* a_parent) = 0;
    virtual Features features() const = 0;
};

/**
 * @short A model that holds a convenient representation of the defined class in the project
 *
 * This model doesn't have much code in it, it mostly acts as a glue between the different
 * nodes and the tree view.
 *
 * The nodes are defined in the namespace @ref ClassModelNodes
 */
class KDEVPLATFORMLANGUAGE_EXPORT ClassModel
    : public QAbstractItemModel
    , public NodesModelInterface
{
    Q_OBJECT

public:
    ClassModel();
    ~ClassModel() override;

public:
    /// Retrieve the DU object related to the specified index.
    /// @note DUCHAINS READER LOCK MUST BE TAKEN!
    KDevelop::DUChainBase* duObjectForIndex(const QModelIndex& a_index);

    /// Call this to retrieve the index for the node associated with the specified id.
    QModelIndex indexForIdentifier(const KDevelop::IndexedQualifiedIdentifier& a_id);

    /// Return the model index associated with the given node.
    QModelIndex index(ClassModelNodes::Node* a_node) const;

    inline void setFeatures(NodesModelInterface::Features features);
    inline NodesModelInterface::Features features() const override { return m_features; }

public Q_SLOTS:
    /// Call this to update the filter string for the search results folder.
    void updateFilterString(const QString& a_newFilterString);

    /// removes the project-specific node
    void removeProjectNode(KDevelop::IProject* project);
    /// adds the project-specific node
    void addProjectNode(KDevelop::IProject* project);

private: // NodesModelInterface overrides
    void nodesLayoutAboutToBeChanged(ClassModelNodes::Node* a_parent) override;
    void nodesLayoutChanged(ClassModelNodes::Node* a_parent) override;
    void nodesAboutToBeRemoved(ClassModelNodes::Node* a_parent, int a_first, int a_last) override;
    void nodesRemoved(ClassModelNodes::Node* a_parent) override;
    void nodesAboutToBeAdded(ClassModelNodes::Node* a_parent, int a_pos, int a_size) override;
    void nodesAdded(ClassModelNodes::Node* a_parent) override;

private:
    /// Main level node - it's usually invisible.
    ClassModelNodes::Node* m_topNode;
    ClassModelNodes::FilteredAllClassesFolder* m_allClassesNode;
    QMap<KDevelop::IProject*, ClassModelNodes::FilteredProjectFolder*> m_projectNodes;
    NodesModelInterface::Features m_features;

public Q_SLOTS:
    /// This slot needs to be attached to collapsed signal in the tree view.
    void collapsed(const QModelIndex& index);
    /// This slot needs to be attached to expanded signal in the tree view.
    void expanded(const QModelIndex& index);

public: // QAbstractItemModel overrides
    QFlags<Qt::ItemFlag> flags(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
};

inline void ClassModel::setFeatures(Features features)
{ m_features = features; }

Q_DECLARE_OPERATORS_FOR_FLAGS(NodesModelInterface::Features)

#endif
