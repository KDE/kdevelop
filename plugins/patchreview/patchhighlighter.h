/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_PATCHHIGHLIGHTER_H
#define KDEVPLATFORM_PLUGIN_PATCHHIGHLIGHTER_H

#include <QObject>
#include <QPoint>
#include <QSet>
#include <QMap>

#include <ktexteditor/movingrangefeedback.h>

namespace Diff2 {
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
class Mark;
class MovingRange;
}

///Delete itself when the document(or textDocument), or Diff-Model is deleted.
class PatchHighlighter : public QObject
{
    Q_OBJECT
public:
    PatchHighlighter( Diff2::DiffModel* model, KDevelop::IDocument* doc, PatchReviewPlugin* plugin ) throw( QString );
    ~PatchHighlighter();
    KDevelop::IDocument* doc();
    QList< KTextEditor::MovingRange* > ranges() const;
private slots:
    void documentDestroyed();
    void aboutToDeleteMovingInterfaceContent( KTextEditor::Document* );
private:

    void addLineMarker( KTextEditor::MovingRange* arg1, Diff2::Difference* arg2 );
    void removeLineMarker( KTextEditor::MovingRange* range );
    QStringList splitAndAddNewlines( const QString& text ) const;
    void performContentChange( KTextEditor::Document* doc, const QStringList& oldLines, const QStringList& newLines, int editLineNumber );

    KTextEditor::MovingRange* rangeForMark(const KTextEditor::Mark& mark);

    void clear();
    QSet< KTextEditor::MovingRange* > m_ranges;
    QMap< KTextEditor::MovingRange*, Diff2::Difference* > m_differencesForRanges;
    KDevelop::IDocument* m_doc;
    PatchReviewPlugin* m_plugin;
    Diff2::DiffModel* m_model;
    bool m_applying;
public slots:
    void markToolTipRequested( KTextEditor::Document*, const KTextEditor::Mark&, QPoint, bool & );
    void showToolTipForMark( QPoint arg1, KTextEditor::MovingRange* arg2, QPair< int, int > highlightMark = qMakePair( -1, -1 ) );
    bool isRemoval( Diff2::Difference* );
    bool isInsertion( Diff2::Difference* );
    void markClicked( KTextEditor::Document*, const KTextEditor::Mark&, bool& );
    void textInserted( KTextEditor::Document* doc, const KTextEditor::Range& range );
    void textRemoved( KTextEditor::Document*, const KTextEditor::Range&, const QString& oldText );
};

#endif
