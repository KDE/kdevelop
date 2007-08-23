/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVDOCUMENTRANGEOBJECT_H
#define KDEVDOCUMENTRANGEOBJECT_H

#include <QtCore/QMutex>

#include <ktexteditor/range.h>
#include <ktexteditor/rangefeedback.h>

#include <documentcursor.h>
#include <documentrange.h>

namespace KDevelop
{

/**
 * Base class for any object which has an associated range of text.
 *
 * This allows text without a currently loaded text editor to be represented.
 */
class KDEVPLATFORMEDITOR_EXPORT DocumentRangeObject : public KTextEditor::SmartRangeWatcher
{
public:
    DocumentRangeObject(KTextEditor::Range* range);
    virtual ~DocumentRangeObject();

    enum RangeOwning{ Own, DontOwn };
    Q_DECLARE_FLAGS( RangeOwnings, RangeOwning )

    /**
     * Sets the text \a range to this object.  If \a ownsRange is false, the range won't be
     * deleted when the object is deleted.
     */
    void setTextRange(KTextEditor::Range* range, RangeOwning ownsRange = Own);

    void setRangeOwning(RangeOwning ownsRange);

    void setRange(const KTextEditor::Range& range);
    ///Returns the text-range. Needs to be a reference for performance-reasons.
    const KTextEditor::Range& textRange() const;
    const DocumentRange textDocRange() const;
    KTextEditor::Range* textRangePtr() const;
    ///If this document's range is a SmartRange, returns it. Else 0.
    KTextEditor::SmartRange* smartRange() const;

    RangeOwning ownsRange() const;
    
    KUrl url() const;
    static KUrl url(const KTextEditor::Range* range);

    bool contains(const DocumentCursor& cursor) const;

    virtual void rangeDeleted(KTextEditor::SmartRange* range);

    /// Take the range from this object. USE WITH CARE!! You must be willing to
    /// immediately delete this range object if you take its range.
    KTextEditor::Range* takeRange();

private:
    Q_DISABLE_COPY(DocumentRangeObject)

    class DocumentRangeObjectPrivate* const d;
};


Q_DECLARE_OPERATORS_FOR_FLAGS( KDevelop::DocumentRangeObject::RangeOwnings )

}
#endif // RANGEOBJECT_H

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

