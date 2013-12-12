/* KDevelop CMake Support
 * 
 * Copyright 2007-2013 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakeedit.h"

#include <KTextEditor/Document>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/use.h>
#include <language/duchain/declaration.h>
#include <language/codegen/applychangeswidget.h>
#include <project/projectmodel.h>
#include "cmakemodelitems.h"

using namespace KDevelop;

namespace CMakeEdit {

void eatLeadingWhitespace(KTextEditor::Document* doc, KTextEditor::Range& eater, const KTextEditor::Range& bounds)
{
    QString text = doc->text(KTextEditor::Range(bounds.start(), eater.start()));
    int newStartLine = eater.start().line(), pos = text.length() - 2; //pos = index before eater.start
    while (pos > 0)
    {
        if (text[pos] == '\n')
            --newStartLine;
        else if (!text[pos].isSpace())
        {
            ++pos;
            break;
        }
        --pos;
    }
    int lastNewLinePos = text.lastIndexOf('\n', pos - 1);
    int newStartCol = lastNewLinePos == -1 ? eater.start().column() + pos :
    pos - lastNewLinePos - 1;
    eater.start().setLine(newStartLine);
    eater.start().setColumn(newStartCol);
}

KTextEditor::Range rangeForText(KTextEditor::Document* doc, const KTextEditor::Range& r, const QString& name)
{
    QString txt=doc->text(r);
    QRegExp match("([\\s]|^)(\\./)?"+QRegExp::escape(name));
    int namepos = match.indexIn(txt);
    int length = match.cap(0).size();
    
    if(namepos == -1)
        return KTextEditor::Range::invalid();
    //QRegExp doesn't support lookbehind asserts, and \b isn't good enough
    //so either match "^" or match "\s" and then +1 here
    if (txt[namepos].isSpace()) {
        ++namepos;
        --length;
    }
    
    KTextEditor::Cursor c(r.start());
    c.setLine(c.line() + txt.left(namepos).count('\n'));
    int lastNewLinePos = txt.lastIndexOf('\n', namepos);
    if (lastNewLinePos < 0)
        c.setColumn(r.start().column() + namepos);
    else
        c.setColumn(namepos - lastNewLinePos - 1);
    
    return KTextEditor::Range(c, KTextEditor::Cursor(c.line(), c.column()+length));
}

bool followUses(KTextEditor::Document* doc, RangeInRevision r, const QString& name, const KUrl& lists, bool add, const QString& replace)
{
    bool ret=false;
    KTextEditor::Range rx;
    if(!add)
        rx=rangeForText(doc, r.castToSimpleRange().textRange(), name);
    
    if(!add && rx.isValid())
    {
        if(replace.isEmpty())
        {
            eatLeadingWhitespace(doc, rx, r.castToSimpleRange().textRange());
            doc->removeText(rx);
        }
        else
            doc->replaceText(rx, replace);
        
        ret=true;
    }
    else
    {
        const IndexedString idxLists(lists);
        KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
        KDevelop::ReferencedTopDUContext topctx=DUChain::self()->chainForDocument(idxLists);
        QList<Declaration*> decls;
        for(int i=0; i<topctx->usesCount(); i++)
        {
            Use u = topctx->uses()[i];
            
            if(!r.contains(u.m_range))
                continue; //We just want the uses in the range, not the whole file
                
                Declaration* d=u.usedDeclaration(topctx);
            
            if(d && d->topContext()->url()==idxLists)
                decls += d;
        }
        
        if(add && decls.isEmpty())
        {
            doc->insertText(r.castToSimpleRange().textRange().start(), ' '+name);
            ret=true;
        }
        else foreach(Declaration* d, decls)
        {
            r.start=d->range().end;
            
            for(int lineNum = r.start.line; lineNum <= r.end.line; lineNum++)
            {
                int endParenIndex = doc->line(lineNum).indexOf(')');
                if(endParenIndex >= 0) {
                    r.end = CursorInRevision(lineNum, endParenIndex);
                    break;
                }
            }
            
            if(!r.isEmpty())
            {
                ret = ret || followUses(doc, r, name, lists, add, replace);
            }
        }
    }
    return ret;
}

QString dotlessRelativeUrl(const KUrl& baseUrl, const KUrl& url)
{
    QString dotlessRelative = KUrl::relativeUrl(baseUrl, url);
    if (dotlessRelative.startsWith("./"))
        dotlessRelative.remove(0, 2);
    return dotlessRelative;
}

QString relativeToLists(const KUrl& listsPath, const KUrl& url)
{
    KUrl listsFolder(listsPath.upUrl());
    listsFolder.adjustPath(KUrl::AddTrailingSlash);
    return dotlessRelativeUrl(listsFolder, url);
}

KUrl afterMoveUrl(const KUrl& origUrl, const KUrl& movedOrigUrl, const KUrl& movedNewUrl)
{
    QString difference = dotlessRelativeUrl(movedOrigUrl, origUrl);
    return KUrl(movedNewUrl, difference);
}

QString itemListspath(const ProjectBaseItem* item)
{
    const DescriptorAttatched *desc = 0;
    if (item->parent()->target())
        desc = dynamic_cast<const DescriptorAttatched*>(item->parent());
    else if (item->type() == ProjectBaseItem::BuildFolder)
        desc = dynamic_cast<const DescriptorAttatched*>(item);
    
    if (!desc)
        return QString();
    return desc->descriptor().filePath;
}

bool itemAffected(const ProjectBaseItem *item, const KUrl &changeUrl)
{
    KUrl listsPath = itemListspath(item);
    if (listsPath.isEmpty())
        return false;
    
    KUrl listsFolder(listsPath.toLocalFile(KUrl::AddTrailingSlash));
    //Who thought it was a good idea to have KUrl::isParentOf return true if the urls are equal?
    return listsFolder.QUrl::isParentOf(changeUrl);
}

QList<ProjectBaseItem*> cmakeListedItemsAffectedByUrlChange(const IProject *proj, const KUrl &url, KUrl rootUrl)
{
    if (rootUrl.isEmpty())
        rootUrl = url;
    
    QList<ProjectBaseItem*> dirtyItems;
    
    QList<ProjectBaseItem*> sameUrlItems = proj->itemsForUrl(url);
    foreach(ProjectBaseItem *sameUrlItem, sameUrlItems)
    {
        if (itemAffected(sameUrlItem, rootUrl))
            dirtyItems.append(sameUrlItem);
        
        foreach(ProjectBaseItem* childItem, sameUrlItem->children())
            dirtyItems.append(cmakeListedItemsAffectedByUrlChange(childItem->project(), childItem->url(), rootUrl));
    }
    return dirtyItems;
}

QList<ProjectBaseItem*> cmakeListedItemsAffectedByItemsChanged(const QList<ProjectBaseItem*> &items)
{
    QList<ProjectBaseItem*> dirtyItems;
    foreach(ProjectBaseItem *item, items)
        dirtyItems.append(cmakeListedItemsAffectedByUrlChange(item->project(), item->url()));
    return dirtyItems;
}

bool changesWidgetRenameFolder(const CMakeFolderItem *folder, const KUrl &newUrl, ApplyChangesWidget *widget)
{
    QString lists = folder->descriptor().filePath;
    widget->addDocuments(IndexedString(lists));
    QString relative(relativeToLists(lists, newUrl));
    KTextEditor::Range range = folder->descriptor().argRange().castToSimpleRange().textRange();
    return widget->document()->replaceText(range, relative);
}

bool changesWidgetRemoveCMakeFolder(const CMakeFolderItem *folder, ApplyChangesWidget *widget)
{
    widget->addDocuments(IndexedString(folder->descriptor().filePath));
    KTextEditor::Range range = folder->descriptor().range().castToSimpleRange().textRange();
    return widget->document()->removeText(range);
}

bool changesWidgetAddFolder(const KUrl &folderUrl, const CMakeFolderItem *toFolder, ApplyChangesWidget *widget)
{
    KUrl lists(toFolder->url(), "CMakeLists.txt");
    QString relative(relativeToLists(lists, folderUrl));
    if (relative.endsWith('/'))
        relative.chop(1);
    QString insert = QString("add_subdirectory(%1)").arg(relative);
    widget->addDocuments(IndexedString(lists));
    return widget->document()->insertLine(widget->document()->lines(), insert);
}

bool changesWidgetMoveTargetFile(const ProjectBaseItem *file, const KUrl &newUrl, ApplyChangesWidget *widget)
{
    const DescriptorAttatched *desc = dynamic_cast<const DescriptorAttatched*>(file->parent());
    if (!desc || desc->descriptor().arguments.isEmpty()) {
        return false;
    }
    RangeInRevision targetRange(desc->descriptor().arguments.first().range().end, desc->descriptor().argRange().end);
    QString listsPath = desc->descriptor().filePath;
    QString newRelative = relativeToLists(listsPath, newUrl);
    QString oldRelative = relativeToLists(listsPath, file->url());
    widget->addDocuments(IndexedString(listsPath));
    return followUses(widget->document(), targetRange, oldRelative, listsPath, false, newRelative);
}

bool changesWidgetAddFileToTarget(const ProjectFileItem *item, const ProjectTargetItem *target, ApplyChangesWidget *widget)
{
    const DescriptorAttatched *desc = dynamic_cast<const DescriptorAttatched*>(target);
    if (!desc || desc->descriptor().arguments.isEmpty()) {
        return false;
    }
    RangeInRevision targetRange(desc->descriptor().arguments.first().range().end, desc->descriptor().range().end);
    QString lists = desc->descriptor().filePath;
    QString relative = relativeToLists(lists, item->url());
    widget->addDocuments(IndexedString(lists));
    return followUses(widget->document(), targetRange, relative, lists, true, QString());
}

bool changesWidgetRemoveFileFromTarget(const ProjectBaseItem *item, ApplyChangesWidget *widget)
{
    const DescriptorAttatched *desc = dynamic_cast<const DescriptorAttatched*>(item->parent());
    if (!desc || desc->descriptor().arguments.isEmpty()) {
        return false;
    }
    RangeInRevision targetRange(desc->descriptor().arguments.first().range().end, desc->descriptor().range().end);
    QString lists = desc->descriptor().filePath;
    QString relative = relativeToLists(lists, item->url());
    widget->addDocuments(IndexedString(lists));
    return followUses(widget->document(), targetRange, relative, lists, false, QString());
}

bool changesWidgetRemoveItems(const QSet<ProjectBaseItem*> &items, ApplyChangesWidget *widget)
{
    foreach(ProjectBaseItem *item, items)
    {
        CMakeFolderItem *folder = dynamic_cast<CMakeFolderItem*>(item);
        if (folder && !changesWidgetRemoveCMakeFolder(folder, widget))
            return false;
        else if (item->parent()->target() && !changesWidgetRemoveFileFromTarget(item, widget))
            return false;
    }
    return true;
}

bool changesWidgetRemoveFilesFromTargets(const QList<ProjectFileItem*> &files, ApplyChangesWidget *widget)
{
    foreach(ProjectBaseItem *file, files)
    {
        Q_ASSERT(file->parent()->target());
        if (!changesWidgetRemoveFileFromTarget(file, widget))
            return false;
    }
    return true;
}

bool changesWidgetAddFilesToTarget(const QList<ProjectFileItem*> &files, const ProjectTargetItem* target, ApplyChangesWidget *widget)
{
    foreach(ProjectFileItem *file, files)
    {
        if (!changesWidgetAddFileToTarget(file, target, widget))
            return false;
    }
    return true;
}

CMakeFolderItem* nearestCMakeFolder(ProjectBaseItem* item)
{
    while(!dynamic_cast<CMakeFolderItem*>(item) && item)
        item = item->parent();
    
    return dynamic_cast<CMakeFolderItem*>(item);
}

}
