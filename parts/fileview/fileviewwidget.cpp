/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfileinfo.h>
#include <qheader.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qregexp.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"

#include "fileviewpart.h"
#include "fileviewconfigwidget.h"
#include "fileviewwidget.h"


// Translations for strings in the project file
static const char *translations[] = {
    I18N_NOOP("Sources"),
    I18N_NOOP("Translations"),
    I18N_NOOP("User Interface"),
    I18N_NOOP("Others")
};


class FileViewGroupItem : public QListViewItem
{
public:
    FileViewGroupItem(QListView *parent, const QString &name, const QString &pattern);
    bool matches(const QString &fileName);

private:
    QStringList patterns;
};


FileViewGroupItem::FileViewGroupItem(QListView *parent, const QString &name, const QString &pattern)
    : QListViewItem(parent, name)
{
    setPixmap(0, SmallIcon("folder"));
    patterns = QStringList::split(';', pattern);
}


bool FileViewGroupItem::matches(const QString &fileName)
{
    QStringList::ConstIterator it;
    for (it = patterns.begin(); it != patterns.end(); ++it) {
        QRegExp re(*it, true, true);
        if (re.match(fileName) == 0)
            return true;
    }

    return false;
}


class FileViewFileItem : public QListViewItem
{
public:
    FileViewFileItem(QListViewItem *parent, const QString &fileName);
    QString fileName() const
    { return fullname; }
    
private:
    static QString extractName(const QString &fileName);
    QString fullname;
};


FileViewFileItem::FileViewFileItem(QListViewItem *parent, const QString &fileName)
    : QListViewItem(parent, extractName(fileName)), fullname(fileName)
{
    setPixmap(0, SmallIcon("document"));
}


QString FileViewFileItem::extractName(const QString &fileName)
{
    QFileInfo fi(fileName);
    return fi.fileName();
}


FileViewWidget::FileViewWidget(FileViewPart *part)
    : KListView(0, "file view widget")
{
    setFocusPolicy(ClickFocus);
    setRootIsDecorated(true);
    setSorting(-1);
    setFrameStyle(Panel | Sunken);
    setLineWidth(2); 
    header()->hide();
    addColumn("");

    connect( this, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL( returnPressed(QListViewItem*)),
             this, SLOT(slotItemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    m_part = part;
}


FileViewWidget::~FileViewWidget()
{}


void FileViewWidget::slotItemExecuted(QListViewItem *item)
{
    if (!item)
        return;

    // Is it a group item?
    if (!item->parent())
        return;

    FileViewFileItem *fvfitem = static_cast<FileViewFileItem*>(item);
    m_part->partController()->editDocument(QString("file://") + fvfitem->fileName());
    m_part->topLevel()->lowerView(this);
}


void FileViewWidget::slotContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;
    KPopupMenu popup(i18n("File View"));
    // TODO: Add, remove groups
    int customizeId = popup.insertItem(i18n("Customize..."));
    popup.insertSeparator();
    if (item->parent()) {
        // Not for group items
        FileViewFileItem *fvfitem = static_cast<FileViewFileItem*>(item);
        FileContext context(fvfitem->fileName());
        m_part->core()->fillContextMenu(&popup, &context);
    }
    
    int res = popup.exec(p);
    if (res == customizeId) {
        KDialogBase dlg(KDialogBase::TreeList, i18n("Customize File Tree"),
                        KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this,
                        "customization dialog");
        QVBox *vbox = dlg.addVBoxPage(i18n("File View"));
        FileViewConfigWidget *w = new FileViewConfigWidget(m_part, vbox, "fileview config widget");
        connect(&dlg, SIGNAL(okClicked()), w, SLOT(accept()));
        dlg.exec();
    }
}


void FileViewWidget::refresh()
{
    while (firstChild())
        delete firstChild();

    QDomElement docEl = m_part->projectDom()->documentElement();
    QDomElement fileviewEl = docEl.namedItem("kdevfileview").toElement();
    QDomElement groupsEl = fileviewEl.namedItem("groups").toElement();

    FileViewGroupItem *lastGroup = 0;
    QDomElement groupEl = groupsEl.firstChild().toElement();
    while (!groupEl.isNull()) {
        if (groupEl.tagName() == "group") {
            FileViewGroupItem *newItem =
                new FileViewGroupItem(this, groupEl.attribute("name"), groupEl.attribute("pattern"));
            if (lastGroup)
                newItem->moveItem(lastGroup);
            lastGroup = newItem;
        }
        groupEl = groupEl.nextSibling().toElement();
    }

    QStringList allFiles = m_part->project()->allSourceFiles();
    QStringList::ConstIterator it;
    for (it = allFiles.begin(); it != allFiles.end(); ++it) {
        QListViewItem *item = firstChild();
        while (item) {
            FileViewGroupItem *fvgitem = static_cast<FileViewGroupItem*>(item);
            if (fvgitem->matches(*it)) {
                (void) new FileViewFileItem(fvgitem, *it);
                break;
            }
            item = item->nextSibling();
        }
    }

    QListViewItem *item = firstChild();
    while (item) {
        item->sortChildItems(0, true);
        item = item->nextSibling();
    }
}


void FileViewWidget::addFile(const QString &fileName)
{
    kdDebug(9017) << "FileView add " << fileName << endl;
    
    QListViewItem *item = firstChild();
    while (item) {
        FileViewGroupItem *fvgitem = static_cast<FileViewGroupItem*>(item);
        if (fvgitem->matches(fileName)) {
            (void) new FileViewFileItem(fvgitem, fileName);
            fvgitem->sortChildItems(0, true);
            break;
        }
        item = item->nextSibling();
    }
}


void FileViewWidget::removeFile(const QString &fileName)
{
    kdDebug(9017) << "FileView remove " << fileName << endl;
    
    QListViewItem *item = firstChild();
    while (item) {
        FileViewGroupItem *fvgitem = static_cast<FileViewGroupItem*>(item);
        QListViewItem *childItem = fvgitem->firstChild();
        while (childItem) {
            FileViewFileItem *fvfitem = static_cast<FileViewFileItem*>(childItem);
            if (fvfitem->fileName() == fileName) {
                delete fvfitem;
                //                fvgitem->sortChildItems(0, true);
                return;
            }
            childItem = childItem->nextSibling();
        }
        item = item->nextSibling();
    }
}

#include "fileviewwidget.moc"
    
