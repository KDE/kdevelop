/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DOCUMENTRANGE_H
#define KDEVPLATFORM_DOCUMENTRANGE_H

#include <language/languageexport.h>
#include <serialization/indexedstring.h>

#include <KTextEditor/Range>

namespace KDevelop {
/**
 * Lightweight object that extends a range with information about the URL to which the range refers.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DocumentRange
    : public KTextEditor::Range
{
public:
    DocumentRange()
    {
    }

    inline DocumentRange(const IndexedString& document, const KTextEditor::Range& range)
        : KTextEditor::Range(range)
        , document(document)
    {
        Q_ASSERT(document.toUrl() == document.toUrl().adjusted(QUrl::NormalizePathSegments));
    }

    inline bool operator==(const DocumentRange& rhs) const
    {
        return document == rhs.document && *static_cast<const KTextEditor::Range*>(this) == rhs;
    }

    static DocumentRange invalid()
    {
        return DocumentRange(IndexedString(), KTextEditor::Range::invalid());
    }

    IndexedString document;
};
}
Q_DECLARE_TYPEINFO(KDevelop::DocumentRange, Q_MOVABLE_TYPE);

namespace QTest {
template <>
inline char* toString(const KDevelop::DocumentRange& documentRange)
{
    auto range = QTest::toString(*static_cast<const KTextEditor::Range*>(&documentRange));
    QByteArray ba = "DocumentRange[range=" + QByteArray::fromRawData(range, strlen(range))
        + ", document=" + documentRange.document.toUrl().toDisplayString().toLatin1() + "]";
    delete[] range;
    return qstrdup(ba.data());
}
}

#endif // KDEVPLATFORM_DOCUMENTRANGE_H
