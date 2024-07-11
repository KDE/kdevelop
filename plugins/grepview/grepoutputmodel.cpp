/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 1999-2001 the KDevelop Team
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2010 Silvère Lestang <silvere.lestang@gmail.com>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grepoutputmodel.h"

#include "debug.h"
#include "greputil.h"
#include "grepviewplugin.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>

#include <KTextEditor/Cursor>
#include <KTextEditor/Document>
#include <KLocalizedString>

#include <QDataStream>
#include <QFontDatabase>
#include <QModelIndex>

using namespace KDevelop;

GrepOutputItem::GrepOutputItem(const DocumentChangePointer& change, const QString &text, bool checkable)
    : QStandardItem(), m_change(change)
{
    setText(text);
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    setCheckable(checkable);
    if(checkable)
        setCheckState(Qt::Checked);
}

GrepOutputItem::GrepOutputItem(const QString& filename, const QString& text, bool checkable)
    : QStandardItem(), m_change(new DocumentChange(IndexedString(filename), KTextEditor::Range::invalid(), QString(), QString()))
{
    setText(text);
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setCheckable(checkable);
    if(checkable)
    {
        setAutoTristate(true);
        setCheckState(Qt::Checked);
    }
}

int GrepOutputItem::lineNumber() const 
{
    // line starts at 0 for cursor but we want to start at 1
    return m_change->m_range.start().line() + 1;
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
        auto *item = static_cast<GrepOutputItem *>(child(i));
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
    
    if(auto *p = static_cast<GrepOutputItem *>(parent()))
    {
        p->refreshState();
    }
}

QVariant GrepOutputItem::data ( int role ) const {
    auto *grepModel = static_cast<GrepOutputModel *>(model());
    if(role == Qt::ToolTipRole && grepModel && isText())
    {
        QString start = text().left(m_change->m_range.start().column()).toHtmlEscaped();
        // show replaced version in tooltip if we are in replace mode
        const QString match = isCheckable() ? grepModel->replacementFor(m_change->m_oldText) : m_change->m_oldText;
        const QString repl  = QLatin1String("<b>") + match.toHtmlEscaped() + QLatin1String("</b>");
        QString end   = text().mid(m_change->m_range.end().column()).toHtmlEscaped();
        const QString toolTip = QLatin1String("<span style=\"white-space:nowrap\">") + QString(start + repl + end).trimmed() + QLatin1String("</span>");
        return toolTip;
    } else if (role == Qt::FontRole) {
        return QFontDatabase::systemFont(QFontDatabase::FixedFont);
    } else {
        return QStandardItem::data(role);
    }
}

GrepOutputItem::~GrepOutputItem()
{}

GrepOutputItem::GrepOutputItem()
    : GrepOutputItem(QString{}, QString{}, false)
{
    qCCritical(PLUGIN_GREPVIEW) << "the unsupported default GrepOutputItem constructor is invoked";
    Q_ASSERT(false);
}

void GrepOutputItem::read(QDataStream& in)
{
    qCCritical(PLUGIN_GREPVIEW) << "a GrepOutputItem is read from a stream, this is not supported";
    Q_ASSERT(false);
    QStandardItem::read(in);
}

void GrepOutputItem::write(QDataStream& out) const
{
    qCCritical(PLUGIN_GREPVIEW) << "a GrepOutputItem is written to a stream, this is not supported";
    Q_ASSERT(false);
    QStandardItem::write(out);
}

///////////////////////////////////////////////////////////////

GrepOutputModel::GrepOutputModel( QObject *parent )
    : QStandardItemModel( parent )
{
    connect(this, &GrepOutputModel::itemChanged,
            this, &GrepOutputModel::updateCheckState);
}

GrepOutputModel::~GrepOutputModel()
{}

void GrepOutputModel::clear()
{
    QStandardItemModel::clear();
    // the above clear() also destroys the root item, so invalidate the pointer
    m_rootItem = nullptr;

    m_fileCount = 0;
    m_matchCount = 0;
}

void GrepOutputModel::setRegExp(const QRegExp& re)
{
    m_regExp = re;
    m_finalUpToDate = false;
}

void GrepOutputModel::setReplacement(const QString& repl)
{
    m_replacement = repl;
    m_finalUpToDate = false;
}

void GrepOutputModel::setReplacementTemplate(const QString& tmpl)
{
    m_replacementTemplate = tmpl;
    m_finalUpToDate = false;
}

QString GrepOutputModel::replacementFor(const QString &text)
{
    if(!m_finalUpToDate)
    {
        m_finalReplacement = substitudePattern(m_replacementTemplate, m_replacement);
        m_finalUpToDate = true;
    }
    return m_regExp.replaceIn(text, m_finalReplacement);
}

void GrepOutputModel::activate( const QModelIndex &idx )
{
    QStandardItem *stditem = itemFromIndex(idx);
    auto *grepitem = dynamic_cast<GrepOutputItem*>(stditem);
    if( !grepitem || !grepitem->isText() )
        return;

    QUrl url = QUrl::fromLocalFile(grepitem->filename());

    int line = grepitem->lineNumber() - 1;
    KTextEditor::Range range( line, 0, line+1, 0);

    // Try to find the actual text range we found during the grep
    IDocument* doc = ICore::self()->documentController()->documentForUrl( url );
    if(!doc)
        doc = ICore::self()->documentController()->openDocument( url, range );
    if(!doc)
        return;
    if (KTextEditor::Document* tdoc = doc->textDocument()) {
        KTextEditor::Range matchRange = grepitem->change()->m_range;
        QString actualText = tdoc->text(matchRange);
        QString expectedText = grepitem->change()->m_oldText;
        if (actualText == expectedText) {
            range = matchRange;
        }
    }

    ICore::self()->documentController()->activateDocument( doc, range );
}

QModelIndex GrepOutputModel::previousItemIndex(const QModelIndex &currentIdx) const
{
    GrepOutputItem* current_item = nullptr;

    if (!currentIdx.isValid()) {
        // no item selected, search recursively for the last item in search results
        QStandardItem *it = item(0,0);
        while (it) {
            QStandardItem *child = it->child( it->rowCount() - 1 );
            if (!child) return it->index();
            it = child;
        }
        return QModelIndex();
    }
    else
        current_item = static_cast<GrepOutputItem*>(itemFromIndex(currentIdx));

    if (current_item->parent() != nullptr) {
        int row = currentIdx.row();

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
    GrepOutputItem* current_item = nullptr;

    if (!currentIdx.isValid()) {
        QStandardItem *it = item(0,0);
        if (!it) return QModelIndex();
        current_item = static_cast<GrepOutputItem*>(it);
    }
    else
        current_item = static_cast<GrepOutputItem*>(itemFromIndex(currentIdx));

    if (current_item->parent() == nullptr) {
        // root item with overview of search results
        if (current_item->rowCount() > 0)
            return nextItemIndex(current_item->child(0)->index());
        else
            return QModelIndex();
    } else {
        int row = currentIdx.row();
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

const GrepOutputItem *GrepOutputModel::getRootItem() const {
    return m_rootItem;
}

bool GrepOutputModel::itemsCheckable() const
{
    return m_itemsCheckable;
}

void GrepOutputModel::makeItemsCheckable(bool checkable)
{
    if(m_itemsCheckable == checkable)
        return;
    if(m_rootItem)
        makeItemsCheckable(checkable, m_rootItem);
    m_itemsCheckable = checkable;
}

void GrepOutputModel::makeItemsCheckable(bool checkable, GrepOutputItem* item)
{
    item->setCheckable(checkable);
    if(checkable)
    {
        item->setCheckState(Qt::Checked);
        if(item->rowCount() && checkable)
            item->setAutoTristate(true);
    }
    for(int row = 0; row < item->rowCount(); ++row)
        makeItemsCheckable(checkable, static_cast<GrepOutputItem*>(item->child(row, 0)));
}

void GrepOutputModel::appendOutputs( const QString &filename, const GrepOutputItem::List &items )
{
    if(items.isEmpty())
        return;
    
    if(rowCount() == 0)
    {
        m_rootItem = new GrepOutputItem(QString(), QString(), m_itemsCheckable);
        appendRow(m_rootItem);
    }
    
    m_fileCount  += 1;
    m_matchCount += items.length();

    const QString matchText = i18np("<b>1</b> match", "<b>%1</b> matches", m_matchCount);
    const QString fileText = i18np("<b>1</b> file", "<b>%1</b> files", m_fileCount);

    m_rootItem->setText(i18nc("%1 is e.g. '4 matches', %2 is e.g. '1 file'", "<b>%1 in %2</b>", matchText, fileText));
    
    QString fnString = i18np("%2: 1 match", "%2: %1 matches",
                             items.length(), ICore::self()->projectController()->prettyFileName(QUrl::fromLocalFile(filename)));

    auto *fileItem = new GrepOutputItem(filename, fnString, m_itemsCheckable);
    m_rootItem->appendRow(fileItem);
    for (const GrepOutputItem& item : items) {
        auto* copy = new GrepOutputItem(item);
        copy->setCheckable(m_itemsCheckable);
        if(m_itemsCheckable)
        {
            copy->setCheckState(Qt::Checked);
            if(copy->rowCount())
                copy->setAutoTristate(true);
        }
        
        fileItem->appendRow(copy);
    }
}

void GrepOutputModel::updateCheckState(QStandardItem* item)
{
    // if we don't disconnect the SIGNAL, the setCheckState will call it in loop
    disconnect(this, &GrepOutputModel::itemChanged, nullptr, nullptr);
    
    // try to update checkstate on non checkable items would make a checkbox appear
    if(item->isCheckable())
    {
        auto *it = static_cast<GrepOutputItem *>(item);
        it->propagateState();
        it->refreshState();
    }

    connect(this, &GrepOutputModel::itemChanged,
            this, &GrepOutputModel::updateCheckState);
}

void GrepOutputModel::doReplacements()
{
    Q_ASSERT(m_rootItem);
    if (!m_rootItem)
        return; // nothing to do, abort

    DocumentChangeSet changeSet;
    changeSet.setFormatPolicy(DocumentChangeSet::NoAutoFormat);
    for(int fileRow = 0; fileRow < m_rootItem->rowCount(); fileRow++)
    {
        auto *file = static_cast<GrepOutputItem *>(m_rootItem->child(fileRow));
        
        for(int matchRow = 0; matchRow < file->rowCount(); matchRow++)
        {
            auto *match = static_cast<GrepOutputItem *>(file->child(matchRow));
            if(match->checkState() == Qt::Checked) 
            {
                DocumentChangePointer change = match->change();
                // setting replacement text based on current replace value
                change->m_newText = replacementFor(change->m_oldText);
                changeSet.addChange(change);
                // this item cannot be checked anymore
                match->setCheckState(Qt::Unchecked);
                match->setEnabled(false);
            }
        }
    }
    
    DocumentChangeSet::ChangeResult result = changeSet.applyAllChanges();
    if(!result.m_success)
    {
        DocumentChangePointer ch = result.m_reasonChange;
        if(ch)
            emit showErrorMessage(i18nc("%1 is the old text, %2 is the new text, %3 is the file path, %4 and %5 are its row and column",
                                        "Failed to replace <b>%1</b> by <b>%2</b> in %3:%4:%5",
                                        ch->m_oldText.toHtmlEscaped(), ch->m_newText.toHtmlEscaped(), ch->m_document.toUrl().toLocalFile(),
                                        ch->m_range.start().line() + 1, ch->m_range.start().column() + 1));
    }
}

void GrepOutputModel::showMessageSlot(IStatus* status, const QString& message)
{
    m_savedMessage = message;
    m_savedIStatus = status;
    showMessageEmit();
}

void GrepOutputModel::showMessageEmit()
{
    emit showMessage(m_savedIStatus, m_savedMessage);
}

bool GrepOutputModel::hasResults()
{
    return(m_matchCount > 0);
}

#include "moc_grepoutputmodel.cpp"
