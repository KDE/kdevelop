/***************************************************************************
 *   Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team              *
 *   bernd@kdevelop.org                                                    *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2010 Silvère Lestang <silvere.lestang@gmail.com>            *
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
#include <QTextDocument>
#include <ktexteditor/cursor.h>
#include <ktexteditor/document.h>
#include <klocale.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>


using namespace KDevelop;

GrepOutputItem::GrepOutputItem(DocumentChangePointer change, const QString &text, bool replace)
    : QStandardItem(), m_change(change)
{
    setText(text);
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    if(replace)
    {
        setCheckable(true);
        setCheckState(Qt::Checked);
        QString replacement = Qt::escape(text.left(change->m_range.start.column)) + // start of line
                            "<b>" + Qt::escape(change->m_newText) + "</b>" +        // replaced part
                            Qt::escape(text.right(text.length() - change->m_range.end.column)); // rest of line
        setToolTip(replacement.trimmed());
    }
}

GrepOutputItem::GrepOutputItem(const QString& filename, const QString& text, bool replace)
    : QStandardItem(), m_change(new DocumentChange(IndexedString(filename), SimpleRange::invalid(), QString(), QString()))
{
    setText(text);
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    if(replace)
    {
        setCheckable(true);
        setTristate(true);
        setCheckState(Qt::Checked);
    }
}

int GrepOutputItem::lineNumber() const 
{
    // line starts at 0 for cursor but we want to start at 1
    return m_change->m_range.start.line + 1;
}

QString GrepOutputItem::filename() const 
{
    return m_change->m_document.str();
}

DocumentChangePointer GrepOutputItem::change() const
{
    return m_change;
}

bool GrepOutputItem::isText() const
{
    return m_change->m_range.isValid();
}

void GrepOutputItem::propagateState()
{
    for(int i = 0; i < rowCount(); i++)
    {
        GrepOutputItem *item = static_cast<GrepOutputItem *>(child(i));
        if(item->isEnabled())
        {
            item->setCheckState(checkState());
            item->propagateState();
        }
    }
}

void GrepOutputItem::refreshState()
{
    if(rowCount() > 0)
    {
        int checked   = 0;
        int unchecked = 0;
        int enabled   = 0; //only enabled items are relevants
        
        for(int i = 0; i < rowCount(); i++)
        {
            QStandardItem *item = child(i);
            if(item->isEnabled())
            {
                enabled += 1;
                switch(child(i)->checkState())
                {
                    case Qt::Checked:
                        checked += 1;
                        break;
                    case Qt::Unchecked:
                        unchecked += 1;
                        break;
                    default: break;
                }
            }
        }
        
        if(enabled == 0)
        {
            setCheckState(Qt::Unchecked);
            setEnabled(false);
        }
        else if(checked == enabled)
        {
            setCheckState(Qt::Checked);
        } 
        else if (unchecked == enabled)
        {
            setCheckState(Qt::Unchecked);
        }
        else
        {
            setCheckState(Qt::PartiallyChecked);
        }
    }
    
    if(GrepOutputItem *p = static_cast<GrepOutputItem *>(parent()))
    {
        p->refreshState();
    }
}

GrepOutputItem::~GrepOutputItem()
{}

///////////////////////////////////////////////////////////////

GrepOutputModel::GrepOutputModel( QObject *parent )
    : QStandardItemModel( parent ), m_regExp(""), rootItem(0), fileCount(0), matchCount(0)
{
    connect(this, SIGNAL(itemChanged(QStandardItem*)),
              this, SLOT(updateCheckState(QStandardItem*)));
}

GrepOutputModel::~GrepOutputModel()
{}

void GrepOutputModel::clear()
{
    QStandardItemModel::clear();
    fileCount = 0;
    matchCount = 0;
}

void GrepOutputModel::setRegExp(const QRegExp& re)
{
    m_regExp = re;
}

void GrepOutputModel::activate( const QModelIndex &idx )
{
    QStandardItem *stditem = itemFromIndex(idx);
    GrepOutputItem *grepitem = dynamic_cast<GrepOutputItem*>(stditem);
    if( !grepitem || !grepitem->isText() )
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

QModelIndex GrepOutputModel::previousItemIndex(const QModelIndex &currentIdx) const
{
    int row = currentIdx.row();
    GrepOutputItem* current_item = dynamic_cast<GrepOutputItem*>(itemFromIndex(currentIdx));
    if(current_item->parent() != 0) //we do nothing if it's the root item
    {
        if(!current_item->isText()) // the item is a file
        {
            int item_row = current_item->row();
            if(item_row > 0)
            {
                int idx_last_item = current_item->parent()->child(item_row - 1)->rowCount() - 1;
                return current_item->parent()->child(item_row - 1)->child(idx_last_item)->index();
            }
        }
        else // the item is a match
        {
            if(row > 0)
                return current_item->parent()->child(row - 1)->index();
            else // we return the index of the last item of the previous file
            {
                int parrent_row = current_item->parent()->row();
                if(parrent_row > 0)
                {
                    int idx_last_item = current_item->parent()->parent()->child(parrent_row - 1)->rowCount() - 1;
                    return current_item->parent()->parent()->child(parrent_row - 1)->child(idx_last_item)->index();
                }
            }
        }
    }
    return currentIdx;
}

QModelIndex GrepOutputModel::nextItemIndex(const QModelIndex &currentIdx) const
{
    int row = currentIdx.row();
    GrepOutputItem* current_item = dynamic_cast<GrepOutputItem*>(itemFromIndex(currentIdx));
    if(current_item->parent() != 0) //we do nothing if it's the root item
    {
        if(!current_item->isText()) // the item is a file
        {
            int item_row = current_item->row();
            if(item_row < current_item->parent()->rowCount())
            {
                return current_item->parent()->child(item_row)->child(0)->index();
            }
        }
        else // the item is a match
        {
            if(row < current_item->parent()->rowCount() - 1)
                return current_item->parent()->child(row + 1)->index();
            else // we return the index of the first item of the next file
            {
                int parrent_row = current_item->parent()->row();
                if(parrent_row < current_item->parent()->parent()->rowCount() - 1)
                {
                    return current_item->parent()->parent()->child(parrent_row + 1)->child(0)->index();
                }
            }
        }
    }
    return currentIdx;
}

void GrepOutputModel::appendOutputs( const QString &filename, const GrepOutputItem::List &items )
{
    if(items.isEmpty())
        return;
    
    bool replace = items[0].isCheckable();  //FIXME : find a cleaner way to get this !
    if(rowCount() == 0)
    {
        rootItem = new GrepOutputItem("", "", replace);
        appendRow(rootItem);
    }
    
    fileCount  += 1;
    matchCount += items.length();
    rootItem->setText(i18n("%1 matches in %2 files", matchCount, fileCount));
    
    QString fnString = i18np("%2 (one match)", "%2 (%1 matches)", items.length(), filename);

    GrepOutputItem *fileItem = new GrepOutputItem(filename, fnString, replace);
    rootItem->appendRow(fileItem);
    //m_tracker.addUrl(KUrl(filename));
    foreach( const GrepOutputItem& item, items )
    {
        fileItem->appendRow(new GrepOutputItem(item));
    }
}

void GrepOutputModel::updateCheckState(QStandardItem* item)
{
    // if we don't disconnect the SIGNAL, the setCheckState will call it in loop
    disconnect(SIGNAL(itemChanged(QStandardItem*)));
    
    // try to update checkstate on non checkable items would make a checkbox appear
    if(item->isCheckable())
    {
        GrepOutputItem *it = static_cast<GrepOutputItem *>(item);
        it->propagateState();
        it->refreshState();
    }

    connect(this, SIGNAL(itemChanged(QStandardItem*)),
              this, SLOT(updateCheckState(QStandardItem*)));
}

void GrepOutputModel::doReplacements()
{
    
    DocumentChangeSet changeSet;
    changeSet.setFormatPolicy(DocumentChangeSet::NoAutoFormat);
    for(int fileRow = 0; fileRow < rootItem->rowCount(); fileRow++)
    {
        GrepOutputItem *file = static_cast<GrepOutputItem *>(rootItem->child(fileRow));
        
        for(int matchRow = 0; matchRow < file->rowCount(); matchRow++)
        {
            GrepOutputItem *match = static_cast<GrepOutputItem *>(file->child(matchRow));
            if(match->checkState() == Qt::Checked) 
            {
                changeSet.addChange(match->change());
                // this item cannot be checked anymore
                match->setCheckState(Qt::Unchecked);
                match->setEnabled(false);
            }
        }
    }
    
    DocumentChangeSet::ChangeResult result = changeSet.applyAllChanges();
    //TODO : really display this
    if(!result.m_success)
    {
        kDebug() << result.m_failureReason;
    }
}


#include "grepoutputmodel.moc"

