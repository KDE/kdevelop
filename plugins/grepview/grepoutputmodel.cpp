/***************************************************************************
 *   Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team              *
 *   bernd@kdevelop.org                                                    *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "grepoutputmodel.h"
#include "grepviewplugin.h"
#include <QModelIndex>
#include <kcolorscheme.h>
#include <ktexteditor/cursor.h>
#include <ktexteditor/document.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

GrepOutputItem::GrepOutputItem(const QString &fileName, const QString &lineNumber,
                   const QString &text, bool showFilename)
    : QStandardItem(), m_fileName(fileName), m_lineNumber(lineNumber)
    , m_text(text), m_showFilename( showFilename )
{
    if( !showFilename )
    {
        QString formattedTxt = lineNumber + ": " + text;
        setText( formattedTxt );
        setData( GrepOutputModel::Text );
    }
    else
    {
        setText( fileName );
        setData( GrepOutputModel::File );
    }
}

GrepOutputItem::~GrepOutputItem()
{}

///////////////////////////////////////////////////////////////

GrepOutputModel::GrepOutputModel( QObject *parent )
    : QStandardItemModel( parent )
    , _lastfilename(QString())
{}
GrepOutputModel::~GrepOutputModel()
{}

void GrepOutputModel::activate( const QModelIndex &idx )
{
    QStandardItem *stditem = itemFromIndex(idx);
    GrepOutputItem *grepitem = dynamic_cast<GrepOutputItem*>(stditem);
    if( !grepitem )
        return;

    KUrl url(grepitem->m_fileName);

    int line = grepitem->m_lineNumber.toInt() - 1;
    KTextEditor::Range range( line, 0, line+1, 0);

    // Translate if the file has changed since grepping
    KTextEditor::Range range2 = range;
//     range = m_tracker.translateRange( url, range );

    kDebug() << "range" << range2 << "translated to " << range;

    // Try to find the actual text range we found during the grep
    if (IDocument* doc = ICore::self()->documentController()->documentForUrl( url )) {
        KTextEditor::Range currentSelection = doc->textSelection();
        if (KTextEditor::Document* tdoc = doc->textDocument()) {
            QString text = tdoc->text( range );
            if (m_regExp.isEmpty())
                m_regExp.setPattern(m_pattern);
            int index = m_regExp.indexIn(text);
            if (index != -1) {
                int addedLines = 0;
                int addedCols = text.lastIndexOf('\n', index);
                if (addedCols == -1)
                    addedCols = index;
                else
                    addedLines = text.left(index).count('\n');

                range = KTextEditor::Range(range.start() + KTextEditor::Cursor(addedLines, addedCols), m_regExp.matchedLength());
            }
        }

        ICore::self()->documentController()->activateDocument( doc, range );

    } else {
        ICore::self()->documentController()->openDocument( url, range );
    }
}

bool GrepOutputModel::isValidIndex( const QModelIndex& idx ) const
{
    return ( idx.isValid() && idx.row() >= 0 && idx.row() < rowCount() && idx.column() == 0 );
}

QModelIndex GrepOutputModel::nextHighlightIndex( const QModelIndex &currentIdx )
{
    int startrow = isValidIndex(currentIdx) ? currentIdx.row() + 1 : 0;
    
    for (int row = 0; row < rowCount(); ++row) {
        int currow = (startrow + row) % rowCount();
        if (GrepOutputItem* grep_item = dynamic_cast<GrepOutputItem*>(item(currow)))
            if (grep_item->data() == Text)
                return index(currow, 0);
    }
    return QModelIndex();
}

QModelIndex GrepOutputModel::previousHighlightIndex( const QModelIndex &currentIdx )
{
    //We have to ensure that startrow is >= rowCount - 1 to get a positive value from the % operation.
    int startrow = rowCount() + (isValidIndex(currentIdx) ? currentIdx.row() : rowCount()) - 1;
    
    for (int row = 0; row < rowCount(); ++row)
    {
        int currow = (startrow - row) % rowCount();
        if (GrepOutputItem* grep_item = dynamic_cast<GrepOutputItem*>(item(currow)))
            if (grep_item->data() == Text)
                return index(currow, 0);
    }
    return QModelIndex();
}

void GrepOutputModel::appendOutputs( const QStringList &lines )
{
    foreach( const QString& line, lines )
    {
        int pos;
        QString filename, linenumber, rest;

        QString str = line;
        if ( (pos = str.indexOf(':')) != -1)
        {
            filename = str.left(pos);
            str.remove( 0, pos+1 );
            if ( ( pos = str.indexOf(':') ) != -1)
            {
                linenumber = str.left(pos);
                str.remove( 0, pos+1 );
                // filename will be displayed only once
                // selecting filename will display line 1 of file,
                // otherwise, line of requested search
                if ( _lastfilename != filename )
                {
                    _lastfilename = filename;
                    appendRow(new GrepOutputItem(filename, "0", filename, true));
                    appendRow(new GrepOutputItem(filename, linenumber, str, false));
                    m_tracker.addUrl(KUrl(filename));
                }
                else
                {
                    appendRow(new GrepOutputItem(filename, linenumber, str, false));
                }
//                 maybeScrollToBottom();
            }
            else
            {
                appendRow( new QStandardItem(line) );
            }
//             m_matchCount++;
        }
    }
}

void GrepOutputModel::appendErrors( const QStringList &lines )
{
    foreach( const QString& line, lines )
        appendRow( new QStandardItem(line) );
}

void GrepOutputModel::slotCompleted()
{
    appendRow( new QStandardItem( "Completed" ) );
}
void GrepOutputModel::slotFailed()
{
    appendRow( new QStandardItem( "Failed" ) );
}

void GrepOutputModel::setRegExp(const QString& regExp)
{
    m_pattern = regExp;
}

#include "grepoutputmodel.moc"

