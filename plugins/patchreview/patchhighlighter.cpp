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

#include "patchhighlighter.h"
#include "libdiff2/difference.h"
#include "patchreview.h"
#include <language/highlighting/colorcache.h>
#include <interfaces/idocument.h>
#include <util/activetooltip.h>
#include <KDebug>
#include <KLocalizedString>
#include <KTextBrowser>
#include <KParts/MainWindow>
#include <KIconEffect>
#include <KColorScheme>
#include <KTextEditor/View>
#include <KMessageBox>
#include <ktexteditor/movinginterface.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <QTextCharFormat>
#include <QWidget>
#include <QTextDocument>
#include <QPointer>

using namespace KDevelop;

namespace
{
QPointer<QWidget> currentTooltip;
KTextEditor::MovingRange* currentTooltipMark;


QSize sizeHintForHtml( QString html, QSize maxSize ) {
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

void PatchHighlighter::showToolTipForMark( QPoint pos, KTextEditor::MovingRange* markRange, QPair<int, int> highlightMark ) {
    if( currentTooltipMark == markRange && currentTooltip )
        return;
    delete currentTooltip;

    //Got the difference
    Diff2::Difference* diff = m_differencesForRanges[markRange];

    QString html;
#if 0
    if( diff->hasConflict() )
        html += i18n( "<b><span style=\"color:red\">Conflict</span></b><br/>" );
#endif

    Diff2::DifferenceStringList lines;

    if( m_plugin->patch()->isAlreadyApplied() && !diff->applied() )
        html += i18n( "<b>Reverted.</b><br/>" );
    else if( !m_plugin->patch()->isAlreadyApplied() && diff->applied() )
        html += i18n( "<b>Applied.</b><br/>" );

    if( diff->applied() ) {
        if( isInsertion( diff ) )
        {
            html += i18n( "<b>Insertion</b><br/>" );
        }else{
            if( isRemoval( diff ) )
                html += i18n( "<b>Removal</b><br/>" );
            html += i18n( "<b>Previous:</b><br/>" );
            lines = diff->sourceLines();
        }
    }else{
        if( isRemoval( diff ) ) {
            html += i18n( "<b>Removal</b><br/>" );
        }else{
            if( isInsertion( diff ) )
                html += i18n( "<b>Insertion</b><br/>" );

            html += i18n( "<b>Alternative:</b><br/>" );

            lines = diff->destinationLines();
        }
    }

    for( int a = 0; a < lines.size(); ++a ) {
        Diff2::DifferenceString* line = lines[a];
        uint currentPos = 0;
        QString string = line->string();

        Diff2::MarkerList markers = line->markerList();

        for( int b = 0; b < markers.size(); ++b ) {
            QString spanText = Qt::escape( string.mid( currentPos, markers[b]->offset() - currentPos ) );
            if( markers[b]->type() == Diff2::Marker::End && ( currentPos != 0 || markers[b]->offset() != static_cast<uint>( string.size() ) ) )
            {
                if( a == highlightMark.first && b == highlightMark.second )
                    html += "<b><span style=\"background:#FF5555\">" + spanText + "</span></b>";
                else
                    html += "<b><span style=\"background:#FFBBBB\">" + spanText + "</span></b>";
            }else{
                html += spanText;
            }
            currentPos = markers[b]->offset();
        }

        html += Qt::escape( string.mid( currentPos, string.length()-currentPos ) );
        html += "<br/>";
    }

    KTextBrowser* browser = new KTextBrowser;
    browser->setPalette( QApplication::palette() );
    browser->setHtml( html );

    int maxHeight = 500;

    browser->setMinimumSize( sizeHintForHtml( html, QSize( ( ICore::self()->uiController()->activeMainWindow()->width()*2 )/3, maxHeight ) ) );
    browser->setMaximumSize( browser->minimumSize() + QSize( 10, 10 ) );
    if( browser->minimumHeight() != maxHeight )
        browser->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin( 0 );
    layout->addWidget( browser );

    KDevelop::ActiveToolTip* tooltip = new KDevelop::ActiveToolTip( ICore::self()->uiController()->activeMainWindow(), pos + QPoint( 5, -browser->sizeHint().height() - 30 ) );
    tooltip->setLayout( layout );
    tooltip->resize( tooltip->sizeHint() + QSize( 10, 10 ) );
    tooltip->move( pos - QPoint( 0, 20 + tooltip->height() ) );
    tooltip->addExtendRect( QRect( pos - QPoint( 15, 15 ), pos + QPoint( 15, 15 ) ) );

    currentTooltip = tooltip;
    currentTooltipMark = markRange;

    ActiveToolTip::showToolTip( tooltip );
}

void PatchHighlighter::markClicked( KTextEditor::Document* doc, KTextEditor::Mark mark, bool& handled ) {
    m_applying = true;
    if( handled )
        return;

    handled = true;

//     TODO: reconsider workaround
//     if( doc->activeView() ) ///This is a workaround, if the cursor is somewhere else, the editor will always jump there when a mark was clicked
//         doc->activeView()->setCursorPosition( KTextEditor::Cursor( mark.line, 0 ) );

    KTextEditor::MovingRange* range = rangeForMark( mark );

    if( range ) {
        QString currentText = doc->text( range->toRange() );
        Diff2::Difference* diff = m_differencesForRanges[range];

        removeLineMarker( range );

        QString sourceText;
        QString targetText;

        for( int a = 0; a < diff->sourceLineCount(); ++a ) {
            sourceText += diff->sourceLineAt( a )->string();
            if( !sourceText.endsWith( "\n" ) )
                sourceText += "\n";
        }

        for( int a = 0; a < diff->destinationLineCount(); ++a ) {
            targetText += diff->destinationLineAt( a )->string();
            if( !targetText.endsWith( "\n" ) )
                targetText += "\n";
        }

        QString replace;
        QString replaceWith;

        if( !diff->applied() ) {
            replace = sourceText;
            replaceWith = targetText;
        }else {
            replace = targetText;
            replaceWith = sourceText;
        }

        if( currentText.simplified() != replace.simplified() ) {
            KMessageBox::error( ICore::self()->uiController()->activeMainWindow(), i18n( "Could not apply the change: Text should be \"%1\", but is \"%2\".", replace, currentText ) );
            return;
        }

        diff->apply( !diff->applied() );

        KTextEditor::Cursor start = range->start().toCursor();
        range->document()->replaceText( range->toRange(), replaceWith );
        uint replaceWithLines = replaceWith.count( '\n' );
        KTextEditor::Range newRange( start, KTextEditor::Cursor(start.line() +  replaceWithLines, start.column()) );

        range->setRange( newRange );

        addLineMarker( range, diff );
    }

    {
        // After applying the change, show the tooltip again, mainly to update an old tooltip
        delete currentTooltip;
        bool h = false;
        markToolTipRequested( doc, mark, QCursor::pos(), h );
    }
    m_applying = false;
}

KTextEditor::MovingRange* PatchHighlighter::rangeForMark( KTextEditor::Mark mark ) {
    for( QMap<KTextEditor::MovingRange*, Diff2::Difference*>::const_iterator it = m_differencesForRanges.constBegin(); it != m_differencesForRanges.constEnd(); ++it ) {
        if( it.key()->start().line() == mark.line )
        {
            return it.key();
        }
    }

    return 0;
}

void PatchHighlighter::markToolTipRequested( KTextEditor::Document*, KTextEditor::Mark mark, QPoint pos, bool& handled ) {
    if( handled )
        return;

    handled = true;

    int myMarksPattern = KTextEditor::MarkInterface::markType22 | KTextEditor::MarkInterface::markType23 | KTextEditor::MarkInterface::markType24 | KTextEditor::MarkInterface::markType25 | KTextEditor::MarkInterface::markType26 | KTextEditor::MarkInterface::markType27;
    if( mark.type & myMarksPattern ) {
        //There is a mark in this line. Show the old text.
        KTextEditor::MovingRange* range = rangeForMark( mark );
        if( range )
            showToolTipForMark( pos, range );
    }
}

bool PatchHighlighter::isInsertion( Diff2::Difference* diff ) {
    return diff->sourceLineCount() == 0;
}

bool PatchHighlighter::isRemoval( Diff2::Difference* diff ) {
    return diff->destinationLineCount() == 0;
}

QStringList PatchHighlighter::splitAndAddNewlines( const QString& text ) const {
    QStringList result = text.split( '\n', QString::KeepEmptyParts );
    for( QStringList::iterator iter = result.begin(); iter != result.end(); ++iter ) {
        iter->append( '\n' );
    }
    if ( !result.isEmpty() ) {
        QString & last = result.last();
        last.remove( last.size() - 1, 1 );
    }
    return result;
}

void PatchHighlighter::performContentChange( KTextEditor::Document* doc, const QStringList& oldLines, const QStringList& newLines, int editLineNumber ) {
    QPair<QList<Diff2::Difference*>, QList<Diff2::Difference*> > diffChange = m_model->linesChanged( oldLines, newLines, editLineNumber );
    QList<Diff2::Difference*> inserted = diffChange.first;
    QList<Diff2::Difference*> removed = diffChange.second;

    // Remove all ranges that are in the same line (the line markers)
    foreach( KTextEditor::MovingRange* r, m_differencesForRanges.keys() ) {
        Diff2::Difference* diff = m_differencesForRanges[r];
        if ( removed.contains( diff ) ) {
            removeLineMarker( r );
            m_ranges.remove( r );
            m_differencesForRanges.remove( r );
            delete r;
            delete diff;
        }
    }

    KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>( doc );
    if ( !moving )
        return;

    foreach( Diff2::Difference* diff, inserted ) {
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

        m_differencesForRanges[r] = diff;
        m_ranges.insert( r );
        addLineMarker( r, diff );
    }
}

void PatchHighlighter::textRemoved( KTextEditor::Document* doc, const KTextEditor::Range& range, const QString& oldText ) {
    if ( m_applying ) { // Do not interfere with patch application
        return;
    }
    kDebug() << "removal range" << range;
    kDebug() << "removed text" << oldText;
    QStringList removedLines = splitAndAddNewlines( oldText );
    int startLine = range.start().line();
    QString remainingLine = doc->line( startLine );
    remainingLine += '\n';
    QString prefix = remainingLine.mid( 0, range.start().column() );
    QString suffix = remainingLine.mid( range.start().column() );
    if ( !removedLines.empty() ) {
        removedLines.first() = prefix + removedLines.first();
        removedLines.last() = removedLines.last() + suffix;
    }
    performContentChange( doc, removedLines, QStringList() << remainingLine, startLine + 1 );
}

void PatchHighlighter::textInserted( KTextEditor::Document* doc, const KTextEditor::Range& range ) {
    if( range == doc->documentRange() )
    {
        kDebug() << "re-doing";
        //The document was loaded / reloaded
        if ( !m_model->differences() )
            return;
        KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>( doc );
        if ( !moving )
            return;

        KTextEditor::MarkInterface* markIface = dynamic_cast<KTextEditor::MarkInterface*>( doc );
        if( !markIface )
            return;

        clear();

        KColorScheme scheme( QPalette::Active );

        QImage tintedInsertion = QIcon::fromTheme( "insert-text" ).pixmap( 16, 16 ).toImage();
        KIconEffect::colorize( tintedInsertion, scheme.foreground( KColorScheme::NegativeText ).color(), 1.0 );
        QImage tintedRemoval = QIcon::fromTheme( "edit-delete" ).pixmap( 16, 16 ).toImage();
        KIconEffect::colorize( tintedRemoval, scheme.foreground( KColorScheme::NegativeText ).color(), 1.0 );
        QImage tintedChange = QIcon::fromTheme( "text-field" ).pixmap( 16, 16 ).toImage();
        KIconEffect::colorize( tintedChange, scheme.foreground( KColorScheme::NegativeText ).color(), 1.0 );

        markIface->setMarkDescription( KTextEditor::MarkInterface::markType22, i18n( "Insertion" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType22, QPixmap::fromImage( tintedInsertion ) );
        markIface->setMarkDescription( KTextEditor::MarkInterface::markType23, i18n( "Removal" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType23, QPixmap::fromImage( tintedRemoval ) );
        markIface->setMarkDescription( KTextEditor::MarkInterface::markType24, i18n( "Change" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType24, QPixmap::fromImage( tintedChange ) );

        markIface->setMarkDescription( KTextEditor::MarkInterface::markType25, i18n( "Insertion" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType25, QIcon::fromTheme( "insert-text" ).pixmap( 16, 16 ) );
        markIface->setMarkDescription( KTextEditor::MarkInterface::markType26, i18n( "Removal" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType26, QIcon::fromTheme( "edit-delete" ).pixmap( 16, 16 ) );
        markIface->setMarkDescription( KTextEditor::MarkInterface::markType27, i18n( "Change" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType27, QIcon::fromTheme( "text-field" ).pixmap( 16, 16 ) );

        for ( Diff2::DifferenceList::const_iterator it = m_model->differences()->constBegin(); it != m_model->differences()->constEnd(); ++it ) {
            Diff2::Difference* diff = *it;
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
                m_ranges << r;

                m_differencesForRanges[r] = *it;

                addLineMarker( r, diff );
            }
        }
    } else {
        if ( m_applying ) { // Do not interfere with patch application
            return;
        }
        kDebug() << "insertion range" << range;
        QString text = doc->text( range );
        kDebug() << "inserted text" << text;
        QStringList insertedLines = splitAndAddNewlines( text );
        int startLine = range.start().line();
        int endLine = range.end().line();
        QString prefix = doc->line( startLine ).mid( 0, range.start().column() );
        QString suffix = doc->line( endLine ).mid( range.end().column() );
        suffix += '\n';
        QString removedLine = prefix + suffix;
        if ( !insertedLines.empty() ) {
            insertedLines.first() = prefix + insertedLines.first();
            insertedLines.last() = insertedLines.last() + suffix;
        }
        performContentChange( doc, QStringList() << removedLine, insertedLines, startLine + 1 );
    }
}

PatchHighlighter::PatchHighlighter( Diff2::DiffModel* model, IDocument* kdoc, PatchReviewPlugin* plugin ) throw( QString )
    : m_doc( kdoc ), m_plugin( plugin ), m_model( model ), m_applying( false ) {
//     connect( kdoc, SIGNAL(destroyed(QObject*)), this, SLOT(documentDestroyed()) );
    connect( kdoc->textDocument(), SIGNAL( textInserted( KTextEditor::Document*, KTextEditor::Range ) ), this, SLOT( textInserted( KTextEditor::Document*, KTextEditor::Range ) ) );
    connect( kdoc->textDocument(), SIGNAL( textRemoved( KTextEditor::Document*, KTextEditor::Range, QString ) ), this, SLOT( textRemoved( KTextEditor::Document*, KTextEditor::Range, QString ) ) );
    connect( kdoc->textDocument(), SIGNAL( destroyed( QObject* ) ), this, SLOT( documentDestroyed() ) );

    KTextEditor::Document* doc = kdoc->textDocument();
    if ( doc->lines() == 0 )
        return;

    connect( doc, SIGNAL( markToolTipRequested( KTextEditor::Document*, KTextEditor::Mark, QPoint, bool & ) ), this, SLOT( markToolTipRequested( KTextEditor::Document*, KTextEditor::Mark, QPoint, bool & ) ) );
    connect( doc, SIGNAL( markClicked( KTextEditor::Document*, KTextEditor::Mark, bool & ) ), this, SLOT( markClicked( KTextEditor::Document*, KTextEditor::Mark, bool & ) ) );
    connect( doc, SIGNAL( aboutToDeleteMovingInterfaceContent( KTextEditor::Document* ) ), this, SLOT( aboutToDeleteMovingInterfaceContent( KTextEditor::Document* ) ) );
    connect( doc, SIGNAL( aboutToInvalidateMovingInterfaceContent( KTextEditor::Document* ) ),
             this, SLOT( aboutToDeleteMovingInterfaceContent( KTextEditor::Document* ) ) );

    textInserted( kdoc->textDocument(), kdoc->textDocument()->documentRange() );
}

void PatchHighlighter::removeLineMarker( KTextEditor::MovingRange* range ) {
    KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>( range->document() );
    if ( !moving )
        return;

    KTextEditor::MarkInterface* markIface = dynamic_cast<KTextEditor::MarkInterface*>( range->document() );
    if( !markIface )
        return;

    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType22 );
    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType23 );
    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType24 );
    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType25 );
    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType26 );
    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType27 );

    // Remove all ranges that are in the same line (the line markers)
    foreach( KTextEditor::MovingRange* r, m_ranges )
    {
        if( r != range && range->contains( r->toRange() ) )
        {
            delete r;
            m_ranges.remove( r );
            m_differencesForRanges.remove( r );
        }
    }
}

void PatchHighlighter::addLineMarker( KTextEditor::MovingRange* range, Diff2::Difference* diff ) {
    KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>( range->document() );
    if ( !moving )
        return;

    KTextEditor::MarkInterface* markIface = dynamic_cast<KTextEditor::MarkInterface*>( range->document() );
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
        QString string = line->string();

        Diff2::MarkerList markers = line->markerList();

        for( int b = 0; b < markers.size(); ++b ) {
            if( markers[b]->type() == Diff2::Marker::End )
            {
                if( currentPos != 0 || markers[b]->offset() != static_cast<uint>( string.size() ) )
                {
                    KTextEditor::MovingRange * r2 = moving->newMovingRange( KTextEditor::Range( KTextEditor::Cursor( a + range->start().line(), currentPos ), KTextEditor::Cursor( a + range->start().line(), markers[b]->offset() ) ) );
                    m_ranges << r2;

                    KTextEditor::Attribute::Ptr t( new KTextEditor::Attribute() );

                    t->setProperty( QTextFormat::BackgroundBrush, QBrush( ColorCache::self()->blendBackground( QColor( 255, 0, 0 ), 70 ) ) );
                    r2->setAttribute( t );
                    r2->setZDepth( -600 );
                }
            }
            currentPos = markers[b]->offset();
        }
    }
}

void PatchHighlighter::clear() {
    if( m_ranges.empty() )
        return;

    KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>( m_doc->textDocument() );
    if ( !moving )
        return;

    KTextEditor::MarkInterface* markIface = dynamic_cast<KTextEditor::MarkInterface*>( m_doc->textDocument() );
    if( !markIface )
        return;

    QHash<int, KTextEditor::Mark*> marks = markIface->marks();
    foreach( int line, marks.keys() ) {
        markIface->removeMark( line, KTextEditor::MarkInterface::markType22 );
        markIface->removeMark( line, KTextEditor::MarkInterface::markType23 );
        markIface->removeMark( line, KTextEditor::MarkInterface::markType24 );
        markIface->removeMark( line, KTextEditor::MarkInterface::markType25 );
        markIface->removeMark( line, KTextEditor::MarkInterface::markType26 );
        markIface->removeMark( line, KTextEditor::MarkInterface::markType27 );
    }

    qDeleteAll( m_ranges );
    m_ranges.clear();
    m_differencesForRanges.clear();
}

PatchHighlighter::~PatchHighlighter() {
    clear();
}

IDocument* PatchHighlighter::doc() {
    return m_doc;
}

void PatchHighlighter::documentDestroyed() {
    kDebug() << "document destroyed";
    m_ranges.clear();
    m_differencesForRanges.clear();
}

void PatchHighlighter::aboutToDeleteMovingInterfaceContent( KTextEditor::Document* ) {
    kDebug() << "about to delete";
    clear();
}

QList< KTextEditor::MovingRange* > PatchHighlighter::ranges() const
{
    return m_differencesForRanges.keys();
}
