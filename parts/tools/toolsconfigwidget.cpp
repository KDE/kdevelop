/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "toolsconfigwidget.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kurldrag.h>

#include "addtooldlg.h"
#include "kapplicationtree.h"


struct ToolsConfigEntry
{
    QString menutext;
    QString cmdline;
    bool isdesktopfile;
    bool captured;
    bool isEmpty() const {
        return ( menutext.isEmpty() && cmdline.isEmpty() );
    }
};


ToolsConfigWidget::ToolsConfigWidget(QWidget *parent, const char *name)
    : ToolsConfigWidgetBase(parent, name)
{
    m_toolsmenuEntries.setAutoDelete(true);
    m_filecontextEntries.setAutoDelete(true);
    m_dircontextEntries.setAutoDelete(true);

    toolsmenuBox->setAcceptDrops(true);
    toolsmenuBox->installEventFilter(this);
    toolsmenuBox->viewport()->setAcceptDrops(true);
    toolsmenuBox->viewport()->installEventFilter(this);

    readConfig();
}


ToolsConfigWidget::~ToolsConfigWidget()
{}


void ToolsConfigWidget::readGroup(const QString &group, QDict<ToolsConfigEntry> *entryDict)
{
    KConfig *config = ToolsFactory::instance()->config();
    config->setGroup("External Tools");
    QStringList list = config->readListEntry(group);

    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        config->setGroup(group + " " + (*it));
        QString cmdline = config->readPathEntry("CommandLine");
        bool isdesktopfile = config->readBoolEntry("DesktopFile");
        bool captured = config->readBoolEntry("Captured");
        ToolsConfigEntry *entry = new ToolsConfigEntry;
        entry->menutext = (*it);
        entry->cmdline = cmdline;
        entry->isdesktopfile = isdesktopfile;
        entry->captured = captured;
        entryDict->insert(*it, entry);
    }
}


void ToolsConfigWidget::storeGroup(const QString &group, const QDict<ToolsConfigEntry> &entryDict)
{
    KConfig *config = ToolsFactory::instance()->config();

    QStringList list;

    QDictIterator<ToolsConfigEntry> it(entryDict);
    for (; it.current(); ++it) {
        ToolsConfigEntry *entry = it.current();
        list << entry->menutext;
        config->setGroup(group + " " + entry->menutext);
        config->writePathEntry("CommandLine", entry->cmdline);
        config->writeEntry("DesktopFile", entry->isdesktopfile);
        config->writeEntry("Captured", entry->captured);
    }

    config->setGroup("External Tools");
    config->writeEntry(group, list);
}



void ToolsConfigWidget::fillListBox(QListBox *lb, const QDict<ToolsConfigEntry> &entryDict)
{
    lb->clear();

    QDictIterator<ToolsConfigEntry> it(entryDict);
    for (; it.current(); ++it) {
        ToolsConfigEntry *entry = it.current();
        if (entry->isdesktopfile) {
            KDesktopFile df(entry->cmdline);
            lb->insertItem(SmallIcon(df.readIcon()), entry->menutext);
        } else {
            lb->insertItem(entry->menutext);
        }
    }
}


bool ToolsConfigWidget::addEntry(ToolsConfigEntry *entry, QDict<ToolsConfigEntry> *entryDict)
{
    QString menutext = entry->menutext;
    if (entryDict->find(menutext)) {
        delete entry;
        KMessageBox::sorry(this, i18n("An entry with this title exists already."));
        return false;
    }

    entryDict->insert(menutext, entry);

    updateListBoxes();
    return true;
}


void ToolsConfigWidget::readConfig()
{
    readGroup("Tool Menu", &m_toolsmenuEntries);
    readGroup("File Context", &m_filecontextEntries);
    readGroup("Dir Context", &m_dircontextEntries);

    updateListBoxes();
}


void ToolsConfigWidget::storeConfig()
{
    storeGroup("Tool Menu", m_toolsmenuEntries);
    storeGroup("File Context", m_filecontextEntries);
    storeGroup("Dir Context", m_dircontextEntries);
}


void ToolsConfigWidget::updateListBoxes()
{
    fillListBox(toolsmenuBox, m_toolsmenuEntries);
    fillListBox(filecontextBox, m_filecontextEntries);
    fillListBox(dircontextBox, m_dircontextEntries);
}


void ToolsConfigWidget::toolsmenuaddClicked()
{
    AddToolDialog dlg(this);
    dlg.setCaption(i18n("Add to Tools Menu"));
    dlg.tree->setFocus();
    while (dlg.exec()) {
        ToolsConfigEntry *entry = new ToolsConfigEntry;
        entry->menutext = dlg.menutextEdit->text();
        entry->cmdline = dlg.getApp().stripWhiteSpace();
        entry->isdesktopfile = false;
        entry->captured = dlg.capturedBox->isChecked();
        if ( entry->isEmpty() )
            delete entry;
        else if (addEntry(entry, &m_toolsmenuEntries))
            return;
    }
}


void ToolsConfigWidget::toolsmenuremoveClicked()
{
    QString menutext = toolsmenuBox->currentText();
    m_toolsmenuEntries.remove(menutext);
    updateListBoxes();
}


void ToolsConfigWidget::filecontextaddClicked()
{
    AddToolDialog dlg(this);
    dlg.setCaption(i18n("Add to File Context Menus"));
    dlg.tree->setFocus();
    while (dlg.exec()) {
        ToolsConfigEntry *entry = new ToolsConfigEntry;
        entry->menutext = dlg.menutextEdit->text();
        entry->cmdline = dlg.getApp().stripWhiteSpace();
        entry->isdesktopfile = false;
        entry->captured = dlg.capturedBox->isChecked();
        if ( entry->isEmpty() )
            delete entry;

        else if (addEntry(entry, &m_filecontextEntries))
            return;
    }
}


void ToolsConfigWidget::filecontextremoveClicked()
{
    QString menutext = filecontextBox->currentText();
    m_filecontextEntries.remove(menutext);
    updateListBoxes();
}


void ToolsConfigWidget::dircontextaddClicked()
{
    AddToolDialog dlg(this);
    dlg.setCaption(i18n("Add to Directory Context Menus"));
    dlg.tree->setFocus();
    if (dlg.exec()) {
        ToolsConfigEntry *entry = new ToolsConfigEntry;
        entry->menutext = dlg.menutextEdit->text();
        entry->cmdline = dlg.getApp().stripWhiteSpace();
        entry->isdesktopfile = false;
        entry->captured = dlg.capturedBox->isChecked();
        if ( entry->isEmpty() )
            delete entry;
        else if (addEntry(entry, &m_dircontextEntries))
            return;
    }
}


void ToolsConfigWidget::dircontextremoveClicked()
{
    QString menutext = dircontextBox->currentText();
    m_dircontextEntries.remove(menutext);
    updateListBoxes();
}


bool ToolsConfigWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::DragEnter || e->type() == QEvent::DragMove) {
        QDragMoveEvent *dme = static_cast<QDragMoveEvent*>(e);
        if (KURLDrag::canDecode(dme))
            dme->accept();
        return true;
    } else if (e->type() == QEvent::Drop) {
        QDropEvent *de = static_cast<QDropEvent*>(e);
        KURL::List fileList;
        if (KURLDrag::decode(de, fileList)) {
            KURL::List::ConstIterator it;
            for (it = fileList.begin(); it != fileList.end(); ++it) {
                if ((*it).isLocalFile() && KDesktopFile::isDesktopFile((*it).path())) {
                    KDesktopFile df((*it).path());
                    ToolsConfigEntry *entry = new ToolsConfigEntry;
                    entry->menutext = df.readName();
                    entry->cmdline = (*it).path();
                    entry->isdesktopfile = true;
                    entry->captured = false;
                    addEntry(entry, &m_toolsmenuEntries);
                }
            }
        }
        return true;
    }

    return ToolsConfigWidgetBase::eventFilter(o, e);
}


void ToolsConfigWidget::accept()
{
    storeConfig();
}
#include "toolsconfigwidget.moc"
