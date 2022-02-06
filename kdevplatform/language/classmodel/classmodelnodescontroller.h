/*
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CLASSMODELNODESCONTROLLER_H
#define KDEVPLATFORM_CLASSMODELNODESCONTROLLER_H

#include <QObject>
#include "../../serialization/indexedstring.h"
#include "../duchain/ducontext.h"

class QTimer;

class ClassModelNodeDocumentChangedInterface
{
public:
    virtual ~ClassModelNodeDocumentChangedInterface();

    /// Called when the registered document is changed.
    virtual void documentChanged(const KDevelop::IndexedString& a_file) = 0;
};

/// This class provides notifications for updates between the different nodes
/// and the various kdevelop sub-systems (such as notification when a DUChain gets
/// updated).
class ClassModelNodesController
    : public QObject
{
    Q_OBJECT

    ClassModelNodesController();
public:
    ~ClassModelNodesController() override;

    static ClassModelNodesController& self();

    /// Register the given class node to receive notifications about its top context changes.
    void registerForChanges(const KDevelop::IndexedString& a_file, ClassModelNodeDocumentChangedInterface* a_node);
    /// Unregister the given class node from further notifications.
    void unregisterForChanges(const KDevelop::IndexedString& a_file, ClassModelNodeDocumentChangedInterface* a_node);

private Q_SLOTS:
    // Files update.
    void updateChangedFiles();

private: // File updates related.
    /// List of updated files we check this list when update timer expires.
    QSet<KDevelop::IndexedString> m_updatedFiles;

    /// Timer for batch updates.
    QTimer* m_updateTimer;

    using FilesMap = QMultiMap<KDevelop::IndexedString, ClassModelNodeDocumentChangedInterface*>;
    /// Maps between monitored files and their class nodes.
    FilesMap m_filesMap;
};

#endif
