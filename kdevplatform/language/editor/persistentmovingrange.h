/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PERSISTENTMOVINGRANGE_H
#define KDEVPLATFORM_PERSISTENTMOVINGRANGE_H

#include <language/languageexport.h>

#include <KTextEditor/Attribute>
#include <KTextEditor/Range>

#include <QExplicitlySharedDataPointer>

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

class KDEVPLATFORMLANGUAGE_EXPORT PersistentMovingRange
    : public QSharedData
{
public:
    using Ptr = QExplicitlySharedDataPointer<PersistentMovingRange>;

    /**
     * Creates a new persistent moving range based on the current revision of the given document
     * */
    PersistentMovingRange(const KTextEditor::Range& range, const IndexedString& document, bool shouldExpand = false);
    ~PersistentMovingRange();

    Q_DISABLE_COPY_MOVE(PersistentMovingRange)

    IndexedString document() const;

    /**
     * Returns the range in the current revision of the document
     */

    KTextEditor::Range range() const;

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
    void setAttribute(const KTextEditor::Attribute::Ptr& attribute);

    /**
     * Whether this range is still valid. The range is invalidated if the document is changed externally,
     * as such a change can not be tracked correctly.
     * */
    bool valid() const;

private:
    PersistentMovingRangePrivate* m_p;
};
}

#endif // KDEVPLATFORM_PERSISTENTMOVINGRANGE_H
