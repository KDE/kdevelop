/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "persistentmovingrangeprivate.h"
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <backgroundparser/backgroundparser.h>

void KDevelop::PersistentMovingRangePrivate::connectTracker()
{
  Q_ASSERT(m_tracker == 0);
  Q_ASSERT(m_movingRange == 0);
  
  m_tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(m_document);
  
  if(m_tracker)
  {
    // Create a moving range
    m_movingRange = m_tracker->documentMovingInterface()->newMovingRange(m_range);
    if (m_shouldExpand)
      m_movingRange->setInsertBehaviors(KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight);
    connect(m_tracker->document(), SIGNAL(aboutToDeleteMovingInterfaceContent(KTextEditor::Document*)), this, SLOT(aboutToDeleteMovingInterfaceContent()));
    connect(m_tracker->document(), SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)), this, SLOT(aboutToInvalidateMovingInterfaceContent()));
    m_movingRange->setAttribute(m_attribte);
    m_movingRange->setZDepth(m_zDepth);
  }
}


void KDevelop::PersistentMovingRangePrivate::disconnectTracker()
{
  Q_ASSERT(m_tracker);
  Q_ASSERT(m_movingRange);
  disconnect(m_tracker->document(), SIGNAL(aboutToDeleteMovingInterfaceContent(KTextEditor::Document*)), this, SLOT(aboutToDeleteMovingInterfaceContent()));
  disconnect(m_tracker->document(), SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)), this, SLOT(aboutToInvalidateMovingInterfaceContent()));
  
  delete m_movingRange;
  m_tracker = 0;
  m_movingRange = 0;
}

void KDevelop::PersistentMovingRangePrivate::aboutToInvalidateMovingInterfaceContent()
{
  if(m_movingRange)
  {
    Q_ASSERT(m_tracker);
    Q_ASSERT(m_movingRange);
    
    m_valid = false; /// @todo More precise tracking: Why is the document being invalidated? Try
                            ///            keeping the range alive. DocumentChangeTracker to the rescue.
    delete m_movingRange;
    m_movingRange = 0;
    m_range = KTextEditor::Range::invalid();
  }
}

void KDevelop::PersistentMovingRangePrivate::aboutToDeleteMovingInterfaceContent()
{
  // The whole document is being closed. Map the range back to the last saved revision, and use that.
  updateRangeFromMoving();
  if(m_tracker->diskRevision())
  {
    if(m_movingRange)
      m_range = m_tracker->diskRevision()->transformFromCurrentRevision(m_range).castToSimpleRange();
  }else{
    m_valid = false;
    m_range = KTextEditor::Range::invalid();
  }
  
  // No need to disconnect, as the document is being deleted. Simply set the referenes to zero.
  delete m_movingRange;
  m_movingRange = 0;
  m_tracker = 0;
}
