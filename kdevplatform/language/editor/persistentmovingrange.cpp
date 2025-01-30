/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "persistentmovingrange.h"
#include "persistentmovingrangeprivate.h"

#include <interfaces/icore.h>
#include <util/foregroundlock.h>

#include <KTextEditor/Document>

KDevelop::PersistentMovingRange::PersistentMovingRange(const KTextEditor::Range& range, const IndexedString& document,
                                                       bool shouldExpand)
    : m_p(new PersistentMovingRangePrivate)
{
    VERIFY_FOREGROUND_LOCKED;
    m_p->m_range = range;
    m_p->m_document = document;
    m_p->m_shouldExpand = shouldExpand;

    m_p->connectTracker();
}

KDevelop::IndexedString KDevelop::PersistentMovingRange::document() const
{
    return m_p->m_document;
}

void KDevelop::PersistentMovingRange::setZDepth(float depth) const
{
    VERIFY_FOREGROUND_LOCKED;
    if (m_p->m_movingRange)
        m_p->m_movingRange->setZDepth(depth);
}

KDevelop::PersistentMovingRange::~PersistentMovingRange()
{
    VERIFY_FOREGROUND_LOCKED;
    delete m_p;
}

KTextEditor::Range KDevelop::PersistentMovingRange::range() const
{
    VERIFY_FOREGROUND_LOCKED;

    m_p->updateRangeFromMoving();
    return m_p->m_range;
}

QString KDevelop::PersistentMovingRange::text() const
{
    VERIFY_FOREGROUND_LOCKED;

    if (m_p->m_movingRange)
        return m_p->m_movingRange->document()->text(m_p->m_movingRange->toRange());

    return QString();
}

bool KDevelop::PersistentMovingRange::valid() const
{
    VERIFY_FOREGROUND_LOCKED;
    return m_p->m_valid;
}

void KDevelop::PersistentMovingRange::setAttribute(const KTextEditor::Attribute::Ptr& attribute)
{
    VERIFY_FOREGROUND_LOCKED;
    if (m_p->m_movingRange)
        m_p->m_movingRange->setAttribute(attribute);
}
