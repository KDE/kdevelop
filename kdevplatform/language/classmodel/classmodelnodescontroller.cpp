/*
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "classmodelnodescontroller.h"
#include "classmodelnode.h"
#include "../duchain/duchainlock.h"
#include "../duchain/duchain.h"
#include "../duchain/classdeclaration.h"
#include <QTimer>

using namespace KDevelop;

ClassModelNodeDocumentChangedInterface::~ClassModelNodeDocumentChangedInterface()
{
}

ClassModelNodesController::ClassModelNodesController()
    : m_updateTimer(new QTimer(this))
{
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, &QTimer::timeout, this, &ClassModelNodesController::updateChangedFiles);
}

ClassModelNodesController::~ClassModelNodesController()
{
}

ClassModelNodesController& ClassModelNodesController::self()
{
    static ClassModelNodesController ret;
    return ret;
}

void ClassModelNodesController::registerForChanges(const KDevelop::IndexedString& a_file,
                                                   ClassModelNodeDocumentChangedInterface* a_node)
{
    m_filesMap.insert(a_file, a_node);
}

void ClassModelNodesController::unregisterForChanges(const KDevelop::IndexedString& a_file,
                                                     ClassModelNodeDocumentChangedInterface* a_node)
{
    m_filesMap.remove(a_file, a_node);
}

void ClassModelNodesController::updateChangedFiles()
{
    // re-parse changed documents.
    // TODO: m_updatedFiles seems no longer set, check again and remove if
    for (const IndexedString& file : std::as_const(m_updatedFiles)) {
        const auto values = m_filesMap.values(file);
        for (ClassModelNodeDocumentChangedInterface* value : values) {
            value->documentChanged(file);
        }
    }

    // Processed all files.
    m_updatedFiles.clear();
}

#include "moc_classmodelnodescontroller.cpp"
