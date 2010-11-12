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
#include "grepoutputview.h"

using namespace KDevelop;

GrepOutputItem::GrepOutputItem(DocumentChangePointer change, const QString &text, bool replace)
    : QStandardItem(), m_change(change)
{
    setText(text);
    setData(Text, Qt::CheckStateRole);
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);
    setCheckState(Qt::Checked);
    if(replace)
    {
        QString replacement = text.left(change->m_range.start.column) +             // start of line
                            change->m_newText +                                     // replaced part
                            text.right(text.length() - change->m_range.end.column); // rest of line
        setToolTip(replacement);
    }
}

GrepOutputItem::GrepOutputItem(const QString& filename, const QString& text)
    : QStandardItem(), m_change(new DocumentChange(IndexedString(filename), SimpleRange::invalid(), QString(), QString()))
{
    setText(text);
    showCollapsed();
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    setCheckState(Qt::Checked);
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
}

void GrepOutputItem::showExpanded()
{
    setData( FileExpanded );
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
    // line starts at 0 for cursor but we want to start at 1
    return m_change->m_range.start.line + 1;
}

QString GrepOutputItem::filename() const {
    return m_change->m_document.str();
}

DocumentChangePointer GrepOutputItem::change() const
{
    return m_change;
}

bool GrepOutputItem::expanded() const {
    return data()==FileExpanded;
}

bool GrepOutputItem::isMatch() const
{
    return m_change->m_range.isValid();
}

GrepOutputItem::~GrepOutputItem()
{}

///////////////////////////////////////////////////////////////

GrepOutputModel::GrepOutputModel( QObject *parent )
    : QStandardItemModel( parent ), m_regExp("")
{
    connect(this, SIGNAL(itemChanged(QStandardItem*)),
              this, SLOT(updateCheckState(QStandardItem*)));
}

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

    GrepOutputItem *fileItem = new GrepOutputItem(filename, fnString);
    appendRow(fileItem);
    //m_tracker.addUrl(KUrl(filename));
    foreach( const GrepOutputItem& item, items )
    {
        fileItem->appendRow(new GrepOutputItem(item));
    }
}

void GrepOutputModel::showErrorMessage( const QString& errorMessage )
{
    ((GrepOutputView*)parent())->setStyleSheet("QLabel { color : red; }");
    ((GrepOutputView*)parent())->setMessage(errorMessage);
}

void GrepOutputModel::showMessage( KDevelop::IStatus* , const QString& message )
{
    ((GrepOutputView*)parent())->setStyleSheet("");
    ((GrepOutputView*)parent())->setMessage(message);
}

void GrepOutputModel::updateCheckState(QStandardItem* item)
{
    // if we don't disconnect the SIGNAL, the setCheckState will call it in loop
    disconnect(SIGNAL(itemChanged(QStandardItem*)));

    if(item->parent() == 0)
    {
        int idx = item->rowCount() - 1;
        
        if(item->checkState() == Qt::Unchecked)
        {
            item->setCheckState (Qt::Checked);
            while(idx >= 0)
            {
                item->child(idx)->setCheckState(Qt::Checked);
                idx--;
            }
        } else if(item->checkState() == Qt::PartiallyChecked)
        {
            item->setCheckState(Qt::Checked);
            while(idx >= 0)
            {
                item->child(idx)->setCheckState(Qt::Checked);
                idx--;
            }
        } else if(item->checkState() == Qt::Checked)
        {
            item->setCheckState(Qt::Unchecked);
            while(idx >= 0)
            {
                item->child(idx)->setCheckState(Qt::Unchecked);
                idx--;
            }
        }
    }
    else
    {
        QStandardItem *parent = item->parent();
        bool checked = false;
        bool unchecked = false;
        int idx = parent->rowCount() - 1;
        while(idx >= 0)
        {
            if(parent->child(idx)->checkState() == Qt::Checked) checked = true;
            else unchecked = true;

            idx--;
        }

        if(checked) unchecked ? parent->setCheckState(Qt::PartiallyChecked) : parent->setCheckState(Qt::Checked);
        else parent->setCheckState(Qt::Unchecked);
    }

    connect(this, SIGNAL(itemChanged(QStandardItem*)),
              this, SLOT(updateCheckState(QStandardItem*)));
}

#include "grepoutputmodel.moc"

