/*
    SPDX-FileCopyrightText: 2006 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PATCHHIGHLIGHTER_H
#define KDEVPLATFORM_PLUGIN_PATCHHIGHLIGHTER_H

#include <KTextEditor/MovingRangeFeedback>

#include <QMap>
#include <QObject>
#include <QPair>
#include <QPoint>

namespace KompareDiff2 {
class Difference;
class DiffModel;
}

class PatchReviewPlugin;

namespace KDevelop
{
class IDocument;
}

namespace KTextEditor
{
class Document;
class Range;
class Cursor;
class Mark;
class MovingRange;
}

///Delete itself when the document(or textDocument), or Diff-Model is deleted.
class PatchHighlighter : public QObject
{
    Q_OBJECT
public:
    PatchHighlighter(KompareDiff2::DiffModel* model, KDevelop::IDocument* doc, PatchReviewPlugin* plugin,
                     bool updatePatchFromEdits);
    ~PatchHighlighter() override;

    QList< KTextEditor::MovingRange* > ranges() const;
private Q_SLOTS:
    void documentReloaded( KTextEditor::Document* );
    void aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*);

private:
    void addLineMarker(KTextEditor::MovingRange* arg1, const KompareDiff2::Difference* arg2);
    void removeLineMarker( KTextEditor::MovingRange* range );
    void performContentChange( KTextEditor::Document* doc, const QStringList& oldLines, const QStringList& newLines, int editLineNumber );

    QPair<KTextEditor::MovingRange*, KompareDiff2::Difference*> rangeForMark(const KTextEditor::Mark& mark) const;

    void clear();
    QMap<KTextEditor::MovingRange*, KompareDiff2::Difference*> m_ranges;
    KDevelop::IDocument* m_doc;
    PatchReviewPlugin* m_plugin;
    KompareDiff2::DiffModel* const m_model;
    bool m_applying;
    static const unsigned int m_allmarks;
public Q_SLOTS:
    void markToolTipRequested( KTextEditor::Document*, const KTextEditor::Mark&, QPoint, bool & );
    void showToolTipForMark(const QPoint& arg1, KTextEditor::MovingRange* arg2);
    bool isRemoval(const KompareDiff2::Difference*);
    bool isInsertion(const KompareDiff2::Difference*);
    void markClicked( KTextEditor::Document*, const KTextEditor::Mark&, bool& );
    void textInserted(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor, const QString& text);
    void newlineInserted(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor);
    void textRemoved( KTextEditor::Document*, const KTextEditor::Range&, const QString& oldText );
    void newlineRemoved(KTextEditor::Document*, int line);
};

#endif
