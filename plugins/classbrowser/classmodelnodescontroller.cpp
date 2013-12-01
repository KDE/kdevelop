/*
 * KDevelop Class Browser
 *
 * Copyright 2009 Lior Mualem <lior.m.kde@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "classmodelnodescontroller.h"
#include "classmodelnode.h"
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainobserver.h>
#include <language/duchain/classdeclaration.h>
#include <QTimer>

using namespace KDevelop;

ClassModelNodeDocumentChangedInterface::~ClassModelNodeDocumentChangedInterface()
{
}

ClassModelNodesController::ClassModelNodesController()
  : m_updateTimer( new QTimer(this) )
{
  m_updateTimer->setSingleShot(true);
  connect( m_updateTimer, SIGNAL(timeout()), this, SLOT(updateChangedFiles()));

  // Get notification for file changes.
  connect(DUChain::self()->notifier(), SIGNAL(branchModified(KDevelop::DUContextPointer)), this, SLOT(branchModified(KDevelop::DUContextPointer)), Qt::QueuedConnection);
}

ClassModelNodesController::~ClassModelNodesController()
{

}

ClassModelNodesController& ClassModelNodesController::self()
{
  static ClassModelNodesController ret;
  return ret;
}

void ClassModelNodesController::registerForChanges(const KDevelop::IndexedString& a_file, ClassModelNodeDocumentChangedInterface* a_node)
{
  m_filesMap.insert(a_file, a_node);
}

void ClassModelNodesController::unregisterForChanges(const KDevelop::IndexedString& a_file, ClassModelNodeDocumentChangedInterface* a_node)
{
  m_filesMap.remove(a_file, a_node);
}

void ClassModelNodesController::branchModified(KDevelop::DUContextPointer context)
{
  DUChainReadLocker readLock(DUChain::lock());

  if ( !context )
    return;

  // Queue the changed file.
  m_updatedFiles.insert(context->url());

  if ( !m_updateTimer->isActive() )
  {
    // Start a delayed update timer.
    m_updateTimer->start(2000);
  }
}

void ClassModelNodesController::updateChangedFiles()
{
  // re-parse changed documents.
  foreach( const IndexedString& file, m_updatedFiles )
    foreach( ClassModelNodeDocumentChangedInterface* value, m_filesMap.values(file) )
    {
      value->documentChanged(file);
    }

  // Processed all files.
  m_updatedFiles.clear();
}

#include "classmodelnodescontroller.moc"
