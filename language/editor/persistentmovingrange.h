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

#ifndef KDEVPLATFORM_PERSISTENTMOVINGRANGE_H
#define KDEVPLATFORM_PERSISTENTMOVINGRANGE_H

#include <QExplicitlySharedDataPointer>
#include <ktexteditor/attribute.h>
#include "simplerange.h"

namespace KDevelop {

class IndexedString;
class PersistentMovingRangePrivate;

/**
 * A range object that is automatically adapted to all changes a user does to a document. The object
 * also survives when the document is opened or closed, as long as the document is only edited from within
 * the application.
 * 
 * This object must only be used from within the foreground, or with the foreground lock held.
 * 
 * @todo The implementation of this object is not finished yet, the range is only persistent until the
 *            document is closed/reloaded/cleared.
 * */

class KDEVPLATFORMLANGUAGE_EXPORT PersistentMovingRange : public QSharedData
{
  public:
    typedef QExplicitlySharedDataPointer<PersistentMovingRange> Ptr;
    
    /**
     * Creates a new persistent moving range based on the current revision of the given document
     * */
    PersistentMovingRange(const SimpleRange& range, const IndexedString& document, bool shouldExpand = false);
    ~PersistentMovingRange();
    
    IndexedString document() const;
    
    /**
     * Returns the range in the current revision of the document
     */
    
    SimpleRange range() const;
    
    /**
     * Changes the z-depth for highlighting (see KTextEditor::MovingRange)
     * */
    void setZDepth(float depth) const;
    
    /**
     * Returns the text contained by the range. Currently only works when the range is open in the editor.
     * */
    QString text() const;
    
    /**
     * Change the highlighting attribute.
     * */
    void setAttribute(KTextEditor::Attribute::Ptr attribute);
    
    /**
     * Whether this range is still valid. The range is invalidated if the document is changed externally,
     * as such a change can not be tracked correctly.
     * */
    bool valid() const;
    
private:
  
    PersistentMovingRange(const PersistentMovingRange& );
    PersistentMovingRange& operator=(const PersistentMovingRange& rhs);
    PersistentMovingRangePrivate* m_p;
};

}

#endif // KDEVPLATFORM_PERSISTENTMOVINGRANGE_H
