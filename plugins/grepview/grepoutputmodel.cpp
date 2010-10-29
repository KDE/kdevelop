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
#include <kicon.h>
#include <klocale.h>

using namespace KDevelop;

GrepOutputItem::GrepOutputItem(const QString &fileName,
                   int lineNumber,
                   const QString &text)
    : QStandardItem(), m_fileName(fileName), m_text(text), m_lineNumber(lineNumber)
{
    if( lineNumber!=-1 )
    {
        QString formattedTxt = QString("  %1: %2").arg(lineNumber).arg(text);
        setText( formattedTxt );
        setData( Text );
    }
    else
    {
        setText(text);
        showCollapsed();
    }
}

bool GrepOutputItem::collapse()
{
    if(isText())
        return false;
    if(collapsed())
        return true;
    QStandardItemModel *model = this->model();
    if(!model)
        return false;
    int takeIdx = this->index().row()+1;
    QList< QList<QStandardItem*> > rows;
    forever
    {
        GrepOutputItem *textitem = dynamic_cast<GrepOutputItem*>(model->item(takeIdx));
        if( !textitem )
            break;
        if(!textitem->isText())
            break;
        rows << model->takeRow(takeIdx);
    }
    foreach(const QList<QStandardItem*> &row, rows)
        this->appendRow(row);
    this->showCollapsed();
    return true;
}

bool GrepOutputItem::expand()
{
    if(isText())
        return false;
    if(expanded())
        return true;
    int curRow = this->index().row()+1;
    while(this->rowCount()!=0)
    {
        QList<QStandardItem*> row = this->takeRow(0);
        this->model()->insertRow(curRow++, row);
    }
    showExpanded();
    return true;
}

void GrepOutputItem::showCollapsed()
{
    setData( FileCollapsed );
    setIcon( KIcon("arrow-right") );
}

void GrepOutputItem::showExpanded()
{
    setData( FileExpanded );
    setIcon( KIcon("arrow-down") );
}

bool GrepOutputItem::toggleView()
{
    if(collapsed())
        expand();
    else if(expanded())
        collapse();
    else
        return false;
    return true;
}

bool GrepOutputItem::collapsed() const {
    return data()==FileCollapsed;
}

int GrepOutputItem::lineNumber() const {
    return m_lineNumber;
}

QString GrepOutputItem::filename() const {
    return m_fileName;
}

bool GrepOutputItem::expanded() const {
    return data()==FileExpanded;
}

GrepOutputItem::~GrepOutputItem()
{}

///////////////////////////////////////////////////////////////

GrepOutputModel::GrepOutputModel( QObject *parent )
    : QStandardItemModel( parent ), m_regExp("")
{}

GrepOutputModel::~GrepOutputModel()
{}

void GrepOutputModel::setRegExp(const QRegExp& re)
{
    m_regExp = re;
}

void GrepOutputModel::activate( const QModelIndex &idx )
{
    QStandardItem *stditem = itemFromIndex(idx);
    GrepOutputItem *grepitem = dynamic_cast<GrepOutputItem*>(stditem);
    if( !grepitem )
        return;
    
    if(grepitem->toggleView())
        return;

    KUrl url(grepitem->filename());

    int line = grepitem->lineNumber() - 1;
    KTextEditor::Range range( line, 0, line+1, 0);

    // Try to find the actual text range we found during the grep
    IDocument* doc = ICore::self()->documentController()->documentForUrl( url );
    if(!doc)
        doc = ICore::self()->documentController()->openDocument( url, range );
    if(!doc)
        return;
    if (KTextEditor::Document* tdoc = doc->textDocument()) {
        QString text = tdoc->line(line);
        int index = m_regExp.indexIn(text);
        if (index!=-1) {
            range.setBothLines(line);
            range.start().setColumn(index);
            range.end().setColumn(index+m_regExp.matchedLength());
            doc->setTextSelection( range );
        }
    }

    ICore::self()->documentController()->activateDocument( doc, range );
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
            if (grep_item->data() == GrepOutputItem::Text)
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
            if (grep_item->data() == GrepOutputItem::Text)
                return index(currow, 0);
    }
    return QModelIndex();
}

void GrepOutputModel::appendOutputs( const QString &filename, const GrepOutputItem::List &items )
{
    QString fnString = i18np("%2 (one match)", "%2 (%1 matches)", items.length(), filename);

    GrepOutputItem *fileItem = new GrepOutputItem(filename, -1, fnString);
    appendRow(fileItem);
    //m_tracker.addUrl(KUrl(filename));
    foreach( const GrepOutputItem& item, items )
    {
        fileItem->appendRow(new GrepOutputItem(item));
    }
}

void GrepOutputModel::showErrorMessage( const QString& errorMessage )
{
    appendRow(new QStandardItem(errorMessage));
}

void GrepOutputModel::showMessage( KDevelop::IStatus* , const QString& message )
{
    appendRow(new QStandardItem(message));
}


#include "grepoutputmodel.moc"

