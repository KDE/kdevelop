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
#include <kconfig.h>
#include "addtooldlg.h"


struct ToolsConfigEntry
{
    QString menutext;
    QString cmdline;
    bool captured;
};


ToolsConfigWidget::ToolsConfigWidget(QWidget *parent, const char *name)
    : ToolsConfigWidgetBase(parent, name)
{
    m_toolsmenuEntries.setAutoDelete(true);
    m_filecontextEntries.setAutoDelete(true);
    m_dircontextEntries.setAutoDelete(true);

    readConfig();
}


ToolsConfigWidget::~ToolsConfigWidget()
{}


void ToolsConfigWidget::readConfig()
{
    KConfig *config = ToolsFactory::instance()->config();
    config->setGroup("External Tools");
    QStringList toolsmenuList = config->readListEntry("Tool Menu");
    QStringList filecontextList = config->readListEntry("File Context");
    QStringList dircontextList = config->readListEntry("Dir Context");

    QStringList::ConstIterator it1;
    for (it1 = toolsmenuList.begin(); it1 != toolsmenuList.end(); ++it1) {
        config->setGroup("Tool Menu " + (*it1));
        QString cmdline = config->readEntry("CommandLine");
        bool captured = config->readBoolEntry("Captured");
        ToolsConfigEntry *entry = new ToolsConfigEntry;
        entry->menutext = (*it1);
        entry->cmdline = cmdline;
        entry->captured = captured;
        m_toolsmenuEntries.append(entry);
    }

    QStringList::ConstIterator it2;
    for (it2 = filecontextList.begin(); it2 != filecontextList.end(); ++it2) {
        config->setGroup("File Context " + (*it2));
        QString cmdline = config->readEntry("CommandLine");
        bool captured = config->readBoolEntry("Captured");
        ToolsConfigEntry *entry = new ToolsConfigEntry;
        entry->menutext = (*it2);
        entry->cmdline = cmdline;
        entry->captured = captured;
        m_filecontextEntries.append(entry);
    }

    QStringList::ConstIterator it3;
    for (it3 = dircontextList.begin(); it3 != dircontextList.end(); ++it3) {
        config->setGroup("Dir Context " + (*it3));
        QString cmdline = config->readEntry("CommandLine");
        bool captured = config->readBoolEntry("Captured");
        ToolsConfigEntry *entry = new ToolsConfigEntry;
        entry->menutext = (*it3);
        entry->cmdline = cmdline;
        entry->captured = captured;
        m_dircontextEntries.append(entry);
    }

    updateListBoxes();
}


void ToolsConfigWidget::storeConfig()
{
    KConfig *config = ToolsFactory::instance()->config();

    QStringList toolsmenuList, filecontextList, dircontextList;
    
    QListIterator<ToolsConfigEntry> it1(m_toolsmenuEntries);
    for (; it1.current(); ++it1) {
        QString menutext = it1.current()->menutext;
        toolsmenuList << menutext;
        config->setGroup("Tool Menu " + menutext);
        config->writeEntry("CommandLine", it1.current()->cmdline);
        config->writeEntry("Captured", it1.current()->captured);
    }

    QListIterator<ToolsConfigEntry> it2(m_filecontextEntries);
    for (; it2.current(); ++it2) {
        QString menutext = it2.current()->menutext;
        filecontextList << menutext;
        config->setGroup("File Context " + menutext);
        config->writeEntry("CommandLine", it2.current()->cmdline);
        config->writeEntry("Captured", it2.current()->captured);
    }

    QListIterator<ToolsConfigEntry> it3(m_dircontextEntries);
    for (; it3.current(); ++it3) {
        QString menutext = it3.current()->menutext;
        dircontextList << menutext;
        config->setGroup("Dir Context " + menutext);
        config->writeEntry("CommandLine", it3.current()->cmdline);
        config->writeEntry("Captured", it3.current()->captured);
    }

    config->setGroup("External Tools");
    config->writeEntry("Tool Menu", toolsmenuList);
    config->writeEntry("File Context", filecontextList);
    config->writeEntry("Dir Context", dircontextList);
}


void ToolsConfigWidget::updateListBoxes()
{
    toolsmenuBox->clear();
    filecontextBox->clear();
    dircontextBox->clear();

    QListIterator<ToolsConfigEntry> it1(m_toolsmenuEntries);
    for (; it1.current(); ++it1)
        toolsmenuBox->insertItem(it1.current()->menutext);
    QListIterator<ToolsConfigEntry> it2(m_filecontextEntries);
    for (; it2.current(); ++it2)
        filecontextBox->insertItem(it2.current()->menutext);
    QListIterator<ToolsConfigEntry> it3(m_dircontextEntries);
    for (; it3.current(); ++it3)
        dircontextBox->insertItem(it3.current()->menutext);
}


void ToolsConfigWidget::toolsmenuaddClicked()
{
    // TODO: Fix code duplication
    // TODO: Avoid entries with the same menutext
    
    AddToolDialog dlg(this);
    dlg.setCaption(i18n("Add to Tools menu"));
    if (dlg.exec()) {
        ToolsConfigEntry *entry = new ToolsConfigEntry;
        entry->menutext = dlg.menutextEdit->text();
        entry->cmdline = dlg.cmdlineEdit->text();
        entry->captured = dlg.capturedBox->isChecked();
        m_toolsmenuEntries.append(entry);
        updateListBoxes();
    }
}


void ToolsConfigWidget::toolsmenuremoveClicked()
{
    QString menutext = toolsmenuBox->currentText();
    QListIterator<ToolsConfigEntry> it(m_toolsmenuEntries);
    for (; it.current(); ++it)
        if (menutext == it.current()->menutext)
            m_toolsmenuEntries.remove(it.current());
    updateListBoxes();
}


void ToolsConfigWidget::filecontextaddClicked()
{
    AddToolDialog dlg(this);
    dlg.setCaption(i18n("Add to file context menus"));
    if (dlg.exec()) {
        ToolsConfigEntry *entry = new ToolsConfigEntry;
        entry->menutext = dlg.menutextEdit->text();
        entry->cmdline = dlg.cmdlineEdit->text();
        entry->captured = dlg.capturedBox->isChecked();
        m_filecontextEntries.append(entry);
        updateListBoxes();
    }
}


void ToolsConfigWidget::filecontextremoveClicked()
{
    QString menutext = filecontextBox->currentText();
    QListIterator<ToolsConfigEntry> it(m_filecontextEntries);
    for (; it.current(); ++it)
        if (menutext == it.current()->menutext)
            m_filecontextEntries.remove(it.current());
    updateListBoxes();
}


void ToolsConfigWidget::dircontextaddClicked()
{
    AddToolDialog dlg(this);
    dlg.setCaption(i18n("Add to directory context menus"));
    if (dlg.exec()) {
        ToolsConfigEntry *entry = new ToolsConfigEntry;
        entry->menutext = dlg.menutextEdit->text();
        entry->cmdline = dlg.cmdlineEdit->text();
        entry->captured = dlg.capturedBox->isChecked();
        m_dircontextEntries.append(entry);
        updateListBoxes();
    }
}


void ToolsConfigWidget::dircontextremoveClicked()
{
    QString menutext = dircontextBox->currentText();
    QListIterator<ToolsConfigEntry> it(m_dircontextEntries);
    for (; it.current(); ++it)
        if (menutext == it.current()->menutext)
            m_dircontextEntries.remove(it.current());
    updateListBoxes();
}


void ToolsConfigWidget::accept()
{
    storeConfig();
}
