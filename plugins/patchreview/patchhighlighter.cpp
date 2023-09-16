/*
    SPDX-FileCopyrightText: 2006 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "patchhighlighter.h"

#ifdef WITH_KOMPAREDIFF2_5_4_OR_NEWER
#include <KompareDiff2/Difference>
#include <KompareDiff2/DiffModel>
#else
#include <libkomparediff2/difference.h>
#include <libkomparediff2/diffmodel.h>
#endif

#include "patchreview.h"
#include "debug.h"

#include <KColorScheme>
#include <KIconEffect>
#include <KLocalizedString>
#include <KParts/MainWindow>
#include <KTextEditor/View>
#include <KTextEditor/Cursor>

#include <KTextEditor/MovingInterface>
#include <KTextEditor/MarkInterface>

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/iuicontroller.h>
#include <language/highlighting/colorcache.h>
#include <util/activetooltip.h>
#include <sublime/message.h>

#include <QApplication>
#include <QPointer>
#include <QTextBrowser>
#include <QTextDocument>
#include <QVBoxLayout>
#include <QWidget>

using namespace KDevelop;

namespace
{
QPointer<QWidget> currentTooltip;
KTextEditor::MovingRange* currentTooltipMark;


QSize sizeHintForHtml( const QString& html, QSize maxSize ) {
    QTextDocument doc;
    doc.setHtml( html );

    QSize ret;
    if( doc.idealWidth() > maxSize.width() ) {
        doc.setPageSize( QSize( maxSize.width(), 30 ) );
        ret.setWidth( maxSize.width() );
    }else{
        ret.setWidth( doc.idealWidth() );
    }
    ret.setHeight( doc.size().height() );
    if( ret.height() > maxSize.height() )
        ret.setHeight( maxSize.height() );
    return ret;
}

}

const unsigned int PatchHighlighter::m_allmarks =
    KTextEditor::MarkInterface::markType22 | KTextEditor::MarkInterface::markType23 |
    KTextEditor::MarkInterface::markType24 | KTextEditor::MarkInterface::markType25 |
    KTextEditor::MarkInterface::markType26 | KTextEditor::MarkInterface::markType27;

void PatchHighlighter::showToolTipForMark(const QPoint& pos, KTextEditor::MovingRange* markRange)
{
    if( currentTooltipMark == markRange && currentTooltip )
        return;
    delete currentTooltip;

    //Got the difference
    Diff2::Difference* diff = m_ranges[markRange];

    QString html;
#if 0
    if( diff->hasConflict() )
        html += i18n( "<b><span style=\"color:red\">Conflict</span></b><br/>" );
#endif

    Diff2::DifferenceStringList lines;

    html += QLatin1String("<b>");
    if( diff->applied() ) {
        if( !m_plugin->patch()->isAlreadyApplied() )
            html += i18n( "Applied.<br/>" );

        if( isInsertion( diff ) ) {
            html += i18n( "Insertion<br/>" );
        } else {
            if( isRemoval( diff ) )
                html += i18n( "Removal<br/>" );
            html += i18n( "Previous:<br/>" );
            lines = diff->sourceLines();
        }
    } else {
        if( m_plugin->patch()->isAlreadyApplied() )
            html += i18n( "Reverted.<br/>" );

        if( isRemoval( diff ) ) {
            html += i18n( "Removal<br/>" );
        } else {
            if( isInsertion( diff ) )
                html += i18n( "Insertion<br/>" );

            html += i18n( "Alternative:<br/>" );

            lines = diff->destinationLines();
        }
    }
    html += QLatin1String("</b>");

    for (auto* line : qAsConst(lines)) {
        uint currentPos = 0;
        const QString& string = line->string();

        const Diff2::MarkerList& markers = line->markerList();

        for (auto* marker : markers) {
            const QString spanText = string.mid( currentPos, marker->offset() - currentPos ).toHtmlEscaped();
            if (marker->type() == Diff2::Marker::End && (currentPos != 0 || marker->offset() != static_cast<uint>( string.size()))) {
                html += QLatin1String("<b><span style=\"background:#FFBBBB\">") + spanText + QLatin1String("</span></b>");
            }else{
                html += spanText;
            }
            currentPos = marker->offset();
        }

        html += string.mid(currentPos, string.length()-currentPos).toHtmlEscaped() + QLatin1String("<br/>");
    }

    auto browser = new QTextBrowser;
    browser->setPalette( QApplication::palette() );
    browser->setHtml( html );

    int maxHeight = 500;

    browser->setMinimumSize( sizeHintForHtml( html, QSize( ( ICore::self()->uiController()->activeMainWindow()->width()*2 )/3, maxHeight ) ) );
    browser->setMaximumSize( browser->minimumSize() + QSize( 10, 10 ) );
    if( browser->minimumHeight() != maxHeight )
        browser->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    auto* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget( browser );

    auto* tooltip = new KDevelop::ActiveToolTip( ICore::self()->uiController()->activeMainWindow(), pos + QPoint( 5, -browser->sizeHint().height() - 30 ) );
    tooltip->setLayout( layout );
    tooltip->resize( tooltip->sizeHint() + QSize( 10, 10 ) );
    tooltip->move( pos - QPoint( 0, 20 + tooltip->height() ) );
    tooltip->setHandleRect( QRect( pos - QPoint( 15, 15 ), pos + QPoint( 15, 15 ) ) );

    currentTooltip = tooltip;
    currentTooltipMark = markRange;

    ActiveToolTip::showToolTip( tooltip );
}

void PatchHighlighter::markClicked( KTextEditor::Document* doc, const KTextEditor::Mark& mark, bool& handled ) {
    if( handled || !(mark.type & m_allmarks) )
        return;

    auto range_diff = rangeForMark(mark);
    m_applying = true;

    if (range_diff.first) {
        handled = true;

        KTextEditor::MovingRange *&range = range_diff.first;
        Diff2::Difference *&diff = range_diff.second;

        QString currentText = doc->text( range->toRange() );

        removeLineMarker( range );

        QString sourceText;
        QString targetText;

        for( int a = 0; a < diff->sourceLineCount(); ++a ) {
            sourceText += diff->sourceLineAt( a )->string();
            if (!sourceText.endsWith(QLatin1Char('\n')))
                sourceText += QLatin1Char('\n');
        }

        for( int a = 0; a < diff->destinationLineCount(); ++a ) {
            targetText += diff->destinationLineAt( a )->string();
            if (!targetText.endsWith(QLatin1Char('\n')))
                targetText += QLatin1Char('\n');
        }

        bool applied = diff->applied();
        QString &replace(applied ? targetText : sourceText);
        QString &replaceWith(applied ? sourceText : targetText);

        if( currentText.simplified() != replace.simplified() ) {
            const QString messageText = i18n("Could not apply the change: Text should be \"%1\", but is \"%2\".", replace, currentText);
            auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
            ICore::self()->uiController()->postMessage(message);

            m_applying = false;
            return;
        }

        diff->apply(!applied);

        KTextEditor::Cursor start = range->start().toCursor();
        range->document()->replaceText( range->toRange(), replaceWith );
        const uint replaceWithLines = replaceWith.count(QLatin1Char('\n'));
        KTextEditor::Range newRange( start, KTextEditor::Cursor(start.line() +  replaceWithLines, start.column()) );

        range->setRange( newRange );

        addLineMarker( range, diff );

        {
            // After applying the change, show the tooltip again, mainly to update an old tooltip
            delete currentTooltip;
            currentTooltip = nullptr;
            bool h = false;
            markToolTipRequested( doc, mark, QCursor::pos(), h );
        }
    }

    m_applying = false;
}

QPair<KTextEditor::MovingRange*, Diff2::Difference*> PatchHighlighter::rangeForMark( const KTextEditor::Mark &mark ) {
    if (!m_applying) {
        for( QMap<KTextEditor::MovingRange*, Diff2::Difference*>::const_iterator it = m_ranges.constBegin(); it != m_ranges.constEnd(); ++it ) {
            if (it.value() && it.key()->start().line() <= mark.line && mark.line <= it.key()->end().line()) {
                return qMakePair(it.key(), it.value());
            }
        }
    }

    return qMakePair(nullptr, nullptr);
}

void PatchHighlighter::markToolTipRequested( KTextEditor::Document*, const KTextEditor::Mark& mark, QPoint pos, bool& handled ) {
    if( handled )
        return;

    if( mark.type & m_allmarks ) {
        //There is a mark in this line. Show the old text.
        auto range = rangeForMark(mark);
        if( range.first ) {
            showToolTipForMark( pos, range.first );
            handled = true;
        }
    }
}

bool PatchHighlighter::isInsertion( Diff2::Difference* diff ) {
    return diff->sourceLineCount() == 0;
}

bool PatchHighlighter::isRemoval( Diff2::Difference* diff ) {
    return diff->destinationLineCount() == 0;
}

void PatchHighlighter::performContentChange( KTextEditor::Document* doc, const QStringList& oldLines, const QStringList& newLines, int editLineNumber ) {
    QPair<QList<Diff2::Difference*>, QList<Diff2::Difference*> > diffChange = m_model->linesChanged( oldLines, newLines, editLineNumber );
    const QList<Diff2::Difference*>& inserted = diffChange.first;
    const QList<Diff2::Difference*>& removed = diffChange.second;

    for (Diff2::Difference* d : removed) {
        const auto sourceLines = d->sourceLines();
        for (Diff2::DifferenceString* s : sourceLines)
            qCDebug(PLUGIN_PATCHREVIEW) << "removed source" << s->string();
        const auto destinationLines = d->destinationLines();
        for (Diff2::DifferenceString* s : destinationLines)
            qCDebug(PLUGIN_PATCHREVIEW) << "removed destination" << s->string();
    }
    for (Diff2::Difference* d : inserted) {
        const auto sourceLines = d->sourceLines();
        for (Diff2::DifferenceString* s : sourceLines)
            qCDebug(PLUGIN_PATCHREVIEW) << "inserted source" << s->string();
        const auto destinationLines = d->destinationLines();
        for (Diff2::DifferenceString* s : destinationLines)
            qCDebug(PLUGIN_PATCHREVIEW) << "inserted destination" << s->string();
    }

    // Remove all ranges that are in the same line (the line markers)
    for (auto it = m_ranges.begin(); it != m_ranges.end();) {
        if (removed.contains(it.value())) {
            KTextEditor::MovingRange* r = it.key();
            removeLineMarker(r); // is altering m_ranges
            it = m_ranges.erase(it);

            delete r;
        } else {
            ++it;
        }
    }
    qDeleteAll(removed);

    auto* moving = qobject_cast<KTextEditor::MovingInterface*>(doc);
    if ( !moving )
        return;

    for (Diff2::Difference* diff : inserted) {
        int lineStart = diff->destinationLineNumber();
        if ( lineStart > 0 ) {
            --lineStart;
        }
        int lineEnd = diff->destinationLineEnd();
        if ( lineEnd > 0 ) {
            --lineEnd;
        }
        KTextEditor::Range newRange( lineStart, 0, lineEnd, 0 );
        KTextEditor::MovingRange * r = moving->newMovingRange( newRange );

        m_ranges[r] = diff;
        addLineMarker( r, diff );
    }
}

void PatchHighlighter::textRemoved( KTextEditor::Document* doc, const KTextEditor::Range& range, const QString& oldText ) {
    if ( m_applying ) { // Do not interfere with patch application
        return;
    }
    qCDebug(PLUGIN_PATCHREVIEW) << "removal range" << range;
    qCDebug(PLUGIN_PATCHREVIEW) << "removed text" << oldText;

    KTextEditor::Cursor cursor = range.start();
    int startLine = cursor.line();
    QStringList removedLines;
    QStringList remainingLines;
    if (startLine > 0) {
        QString above = doc->line(--startLine);
        removedLines << above;
        remainingLines << above;
    }
    const QString changed = doc->line(cursor.line()) + QLatin1Char('\n');
    removedLines << QStringView(changed).mid(0, cursor.column()) + oldText + QStringView(changed).mid(cursor.column());
    remainingLines << changed;
    if (doc->documentRange().end().line() > cursor.line()) {
        QString below = doc->line(cursor.line() + 1);
        removedLines << below;
        remainingLines << below;
    }

    performContentChange(doc, removedLines, remainingLines, startLine + 1);
}

void PatchHighlighter::newlineRemoved(KTextEditor::Document* doc, int line) {
    if ( m_applying ) { // Do not interfere with patch application
        return;
    }
    qCDebug(PLUGIN_PATCHREVIEW) << "remove newline" << line;

    KTextEditor::Cursor cursor = m_doc->cursorPosition();

    int startLine = line - 1;
    QStringList removedLines;
    QStringList remainingLines;
    if (startLine > 0) {
        QString above = doc->line(--startLine);
        removedLines << above;
        remainingLines << above;
    }
    QString changed = doc->line(line - 1);
    if (cursor.line() == line - 1) {
        removedLines << changed.mid(0, cursor.column());
        removedLines << changed.mid(cursor.column());
    } else {
        removedLines << changed;
        removedLines << QString();
    }
    remainingLines << changed;
    if (doc->documentRange().end().line() >= line) {
        QString below = doc->line(line);
        removedLines << below;
        remainingLines << below;
    }

    performContentChange(doc, removedLines, remainingLines, startLine + 1);
}

void PatchHighlighter::documentReloaded(KTextEditor::Document* doc)
{
    qCDebug(PLUGIN_PATCHREVIEW) << "re-doing";
    //The document was loaded / reloaded
    if ( !m_model->differences() )
        return;
    auto* moving = qobject_cast<KTextEditor::MovingInterface*>(doc);
    if ( !moving )
        return;

    auto* markIface = qobject_cast<KTextEditor::MarkInterface*>(doc);
    if( !markIface )
        return;

    clear();

    constexpr int markPixmapSize = 32;
    KColorScheme scheme( QPalette::Active );

    QImage tintedInsertion = QIcon::fromTheme(QStringLiteral("insert-text")).pixmap(markPixmapSize, markPixmapSize).toImage();
    KIconEffect::colorize( tintedInsertion, scheme.foreground( KColorScheme::NegativeText ).color(), 1.0 );
    QImage tintedRemoval = QIcon::fromTheme(QStringLiteral("edit-delete")).pixmap(markPixmapSize, markPixmapSize).toImage();
    KIconEffect::colorize( tintedRemoval, scheme.foreground( KColorScheme::NegativeText ).color(), 1.0 );
    QImage tintedChange = QIcon::fromTheme(QStringLiteral("text-field")).pixmap(markPixmapSize, markPixmapSize).toImage();
    KIconEffect::colorize( tintedChange, scheme.foreground( KColorScheme::NegativeText ).color(), 1.0 );

    markIface->setMarkDescription( KTextEditor::MarkInterface::markType22, i18nc("@item", "Insertion") );
    markIface->setMarkPixmap( KTextEditor::MarkInterface::markType22, QPixmap::fromImage( tintedInsertion ) );
    markIface->setMarkDescription( KTextEditor::MarkInterface::markType23, i18nc("@item", "Removal") );
    markIface->setMarkPixmap( KTextEditor::MarkInterface::markType23, QPixmap::fromImage( tintedRemoval ) );
    markIface->setMarkDescription( KTextEditor::MarkInterface::markType24, i18nc("@item", "Change") );
    markIface->setMarkPixmap( KTextEditor::MarkInterface::markType24, QPixmap::fromImage( tintedChange ) );

    markIface->setMarkDescription( KTextEditor::MarkInterface::markType25, i18nc("@item", "Insertion" ) );
    markIface->setMarkPixmap(KTextEditor::MarkInterface::markType25, QIcon::fromTheme(QStringLiteral("insert-text")).pixmap(markPixmapSize, markPixmapSize));
    markIface->setMarkDescription( KTextEditor::MarkInterface::markType26, i18nc("@item", "Removal") );
    markIface->setMarkPixmap(KTextEditor::MarkInterface::markType26, QIcon::fromTheme(QStringLiteral("edit-delete")).pixmap(markPixmapSize, markPixmapSize));
    markIface->setMarkDescription( KTextEditor::MarkInterface::markType27, i18nc("@item", "Change") );
    markIface->setMarkPixmap(KTextEditor::MarkInterface::markType27, QIcon::fromTheme(QStringLiteral("text-field")).pixmap(markPixmapSize, markPixmapSize));

    for (Diff2::Difference* diff : qAsConst(*m_model->differences())) {
        int line, lineCount;
        Diff2::DifferenceStringList lines;

        if( diff->applied() ) {
            line = diff->destinationLineNumber();
            lineCount = diff->destinationLineCount();
            lines = diff->destinationLines();
        } else {
            line = diff->sourceLineNumber();
            lineCount = diff->sourceLineCount();
            lines = diff->sourceLines();
        }

        if ( line > 0 )
            line -= 1;

        KTextEditor::Cursor c( line, 0 );
        KTextEditor::Cursor endC( line + lineCount, 0 );
        if ( doc->lines() <= c.line() )
            c.setLine( doc->lines() - 1 );
        if ( doc->lines() <= endC.line() )
            endC.setLine( doc->lines() );

        if ( endC.isValid() && c.isValid() ) {
            KTextEditor::MovingRange * r = moving->newMovingRange( KTextEditor::Range( c, endC ) );
            m_ranges[r] = diff;
            addLineMarker( r, diff );
        }
    }
}

void PatchHighlighter::textInserted(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor, const QString& text) {
    if ( m_applying ) { // Do not interfere with patch application
        return;
    }

    int startLine = cursor.line();
    int endColumn = cursor.column() + text.length();

    qCDebug(PLUGIN_PATCHREVIEW) << "insertion range" <<
        KTextEditor::Range(cursor, KTextEditor::Cursor(startLine, endColumn));
    qCDebug(PLUGIN_PATCHREVIEW) << "inserted text" << text;

    QStringList removedLines;
    QStringList insertedLines;
    if (startLine > 0) {
        const QString above = doc->line(--startLine) + QLatin1Char('\n');
        removedLines << above;
        insertedLines << above;
    }
    const QString changed = doc->line(cursor.line()) + QLatin1Char('\n');
    removedLines << QStringView(changed).mid(0, cursor.column()) + QStringView(changed).mid(endColumn);
    insertedLines << changed;
    if (doc->documentRange().end().line() > cursor.line()) {
        const QString below = doc->line(cursor.line() + 1) + QLatin1Char('\n');
        removedLines << below;
        insertedLines << below;
    }

    performContentChange(doc, removedLines, insertedLines, startLine + 1);
}

void PatchHighlighter::newlineInserted(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor)
{
    if ( m_applying ) { // Do not interfere with patch application
        return;
    }
    qCDebug(PLUGIN_PATCHREVIEW) << "newline range" <<
        KTextEditor::Range(cursor, KTextEditor::Cursor(cursor.line() + 1, 0));

    int startLine = cursor.line();
    QStringList removedLines;
    QStringList insertedLines;
    if (startLine > 0) {
        const QString above = doc->line(--startLine) + QLatin1Char('\n');
        removedLines << above;
        insertedLines << above;
    }
    insertedLines << QStringLiteral("\n");
    if (doc->documentRange().end().line() > cursor.line()) {
        const QString below = doc->line(cursor.line() + 1) + QLatin1Char('\n');
        removedLines << below;
        insertedLines << below;
    }

    performContentChange(doc, removedLines, insertedLines, startLine + 1);
}

PatchHighlighter::PatchHighlighter( Diff2::DiffModel* model, IDocument* kdoc, PatchReviewPlugin* plugin, bool updatePatchFromEdits )
    : m_doc( kdoc ), m_plugin( plugin ), m_model( model ), m_applying( false ) {
    KTextEditor::Document* doc = kdoc->textDocument();
//     connect( kdoc, SIGNAL(destroyed(QObject*)), this, SLOT(documentDestroyed()) );
    if (updatePatchFromEdits) {
        connect(doc, &KTextEditor::Document::textInserted, this, &PatchHighlighter::textInserted);
        connect(doc, &KTextEditor::Document::lineWrapped, this, &PatchHighlighter::newlineInserted);
        connect(doc, &KTextEditor::Document::textRemoved, this, &PatchHighlighter::textRemoved);
        connect(doc, &KTextEditor::Document::lineUnwrapped, this, &PatchHighlighter::newlineRemoved);
    }
    connect(doc, &KTextEditor::Document::reloaded, this, &PatchHighlighter::documentReloaded);
    connect(doc, &KTextEditor::Document::destroyed, this, &PatchHighlighter::documentDestroyed);

    if ( doc->lines() == 0 )
        return;

    if (qobject_cast<KTextEditor::MarkInterface*>(doc)) {
        //can't use new signal/slot syntax here, MarkInterface is not a QObject
        connect(doc, SIGNAL(markToolTipRequested(KTextEditor::Document*,KTextEditor::Mark,QPoint,bool&)),
                this, SLOT(markToolTipRequested(KTextEditor::Document*,KTextEditor::Mark,QPoint,bool&)));
        connect(doc, SIGNAL(markClicked(KTextEditor::Document*,KTextEditor::Mark,bool&)),
                this, SLOT(markClicked(KTextEditor::Document*,KTextEditor::Mark,bool&)));
    }
    if (qobject_cast<KTextEditor::MovingInterface*>(doc)) {
        //can't use new signal/slot syntax here, MovingInterface is not a QObject
        connect(doc, SIGNAL(aboutToDeleteMovingInterfaceContent(KTextEditor::Document*)),
                this, SLOT(aboutToDeleteMovingInterfaceContent(KTextEditor::Document*)));
        connect(doc, SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)),
                this, SLOT(aboutToDeleteMovingInterfaceContent(KTextEditor::Document*)));
    }

    documentReloaded(doc);
}

void PatchHighlighter::removeLineMarker( KTextEditor::MovingRange* range ) {
    auto* moving = qobject_cast<KTextEditor::MovingInterface*>(range->document());
    if ( !moving )
        return;

    auto* markIface = qobject_cast<KTextEditor::MarkInterface*>(range->document());
    if( !markIface )
        return;

    for (int line = range->start().line(); line <= range->end().line(); ++line) {
        markIface->removeMark(line, m_allmarks);
    }

    // Remove all ranges that are in the same line (the line markers)
    for (auto it = m_ranges.begin(); it != m_ranges.end();) {
        if (it.key() != range && range->overlaps(it.key()->toRange())) {
            delete it.key();
            it = m_ranges.erase(it);
        } else {
            ++it;
        }
    }
}

void PatchHighlighter::addLineMarker( KTextEditor::MovingRange* range, Diff2::Difference* diff ) {
    auto* moving = qobject_cast<KTextEditor::MovingInterface*>(range->document());
    if ( !moving )
        return;

    auto* markIface = qobject_cast<KTextEditor::MarkInterface*>(range->document());
    if( !markIface )
        return;

    KTextEditor::Attribute::Ptr t( new KTextEditor::Attribute() );

    bool isOriginalState = diff->applied() == m_plugin->patch()->isAlreadyApplied();

    if( isOriginalState ) {
        t->setProperty( QTextFormat::BackgroundBrush, QBrush( ColorCache::self()->blendBackground( QColor( 0, 255, 255 ), 20 ) ) );
    }else{
        t->setProperty( QTextFormat::BackgroundBrush, QBrush( ColorCache::self()->blendBackground( QColor( 255, 0, 255 ), 20 ) ) );
    }
    range->setAttribute( t );
    range->setZDepth( -500 );

    KTextEditor::MarkInterface::MarkTypes mark;

    if( isOriginalState ) {
        mark = KTextEditor::MarkInterface::markType27;

        if( isInsertion( diff ) )
            mark = KTextEditor::MarkInterface::markType25;
        if( isRemoval( diff ) )
            mark = KTextEditor::MarkInterface::markType26;
    }else{
        mark = KTextEditor::MarkInterface::markType24;

        if( isInsertion( diff ) )
            mark = KTextEditor::MarkInterface::markType22;
        if( isRemoval( diff ) )
            mark = KTextEditor::MarkInterface::markType23;
    }

    markIface->addMark( range->start().line(), mark );

    Diff2::DifferenceStringList lines;
    if( diff->applied() )
        lines = diff->destinationLines();
    else
        lines = diff->sourceLines();

    for( int a = 0; a < lines.size(); ++a ) {
        Diff2::DifferenceString* line = lines[a];
        int currentPos = 0;
        const uint lineLength = static_cast<uint>(line->string().size());

        const Diff2::MarkerList& markers = line->markerList();

        for (auto* marker : markers) {
            if (marker->type() == Diff2::Marker::End) {
                if (currentPos != 0 || marker->offset() != lineLength) {
                    KTextEditor::MovingRange* r2 = moving->newMovingRange( KTextEditor::Range( KTextEditor::Cursor( a + range->start().line(), currentPos ), KTextEditor::Cursor( a + range->start().line(), marker->offset() ) ) );
                    m_ranges[r2] = nullptr;

                    KTextEditor::Attribute::Ptr t( new KTextEditor::Attribute() );

                    t->setProperty( QTextFormat::BackgroundBrush, QBrush( ColorCache::self()->blendBackground( QColor( 255, 0, 0 ), 70 ) ) );
                    r2->setAttribute( t );
                    r2->setZDepth( -600 );
                }
            }
            currentPos = marker->offset();
        }
    }
}

void PatchHighlighter::clear() {
    if( m_ranges.empty() )
        return;

    auto* moving = qobject_cast<KTextEditor::MovingInterface*>(m_doc->textDocument());
    if ( !moving )
        return;

    auto* markIface = qobject_cast<KTextEditor::MarkInterface*>(m_doc->textDocument());
    if( !markIface )
        return;

    const auto lines = markIface->marks().keys();
    for (int line : lines) {
        markIface->removeMark( line, m_allmarks );
    }

    // Diff is taking care of its own objects (except removed ones)
    qDeleteAll( m_ranges.keys() );
    m_ranges.clear();
}

PatchHighlighter::~PatchHighlighter() {
    clear();
}

IDocument* PatchHighlighter::doc() {
    return m_doc;
}

void PatchHighlighter::documentDestroyed() {
    qCDebug(PLUGIN_PATCHREVIEW) << "document destroyed";
    m_ranges.clear();
}

void PatchHighlighter::aboutToDeleteMovingInterfaceContent( KTextEditor::Document* ) {
    qCDebug(PLUGIN_PATCHREVIEW) << "about to delete";
    clear();
}

QList< KTextEditor::MovingRange* > PatchHighlighter::ranges() const
{
    return m_ranges.keys();
}

#include "moc_patchhighlighter.cpp"
