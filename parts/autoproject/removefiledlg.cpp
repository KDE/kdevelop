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

#include <qcheckbox.h>
#include <qfile.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <kbuttonbox.h>
#include <kdialog.h>

#include "misc.h"
#include "autoprojectwidget.h"
#include "removefiledlg.h"


static bool fileListContains(const QList<FileItem> &list, const QString &name)
{
    QListIterator<FileItem> it(list);
    for (; it.current(); ++it)
        if ((*it)->text(0) == name)
            return true;
    return false;
}


RemoveFileDialog::RemoveFileDialog(AutoProjectWidget *widget, SubprojectItem *spitem,
                                   TargetItem *item, const QString &filename,
                                   QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Remove file from target"));

    QStringList targets;
    
    QListIterator<TargetItem> it(spitem->targets);
    for (; it.current(); ++it)
        if (fileListContains((*it)->sources, filename))
            targets.append((*it)->name);

    if (targets.count() > 1)
        removefromtargets_box = new QCheckBox(i18n("The file %1 is still used by the following targets:\n%2\n"
                                                   "Remove it from all of them?").arg(filename).arg(targets.join("\n")), this);
    else
        removefromtargets_box = 0;

    removefromdisk_box = new QCheckBox(i18n("Remove from disk"), this);
        
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok_button = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel_button = buttonbox->addButton(i18n("Cancel"));
    ok_button->setDefault(true);
    connect( ok_button, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel_button, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();

    QVBoxLayout *layout = new QVBoxLayout(this, 2*KDialog::marginHint(), KDialog::spacingHint());
    if (removefromtargets_box)
        layout->addWidget(removefromtargets_box);
    layout->addWidget(removefromdisk_box);
    layout->addWidget(frame, 0);
    layout->addWidget(buttonbox, 0);

    m_widget = widget;
    subProject = spitem;
    target = item;
    fileName = filename;
}


RemoveFileDialog::~RemoveFileDialog()
{}


void RemoveFileDialog::accept()
{
    QMap<QCString,QCString> replaceMap;
    
    if (removefromtargets_box && removefromtargets_box->isChecked()) {
        QListIterator<TargetItem> it(subProject->targets);
        for (; it.current(); ++it) {
            if ((*it) != target && fileListContains((*it)->sources, fileName)) {
                FileItem *fitem = static_cast<FileItem*>((*it)->firstChild());
                while (fitem) {
                    FileItem *nextitem = static_cast<FileItem*>(fitem->nextSibling());
                    if (fitem->text(0) == fileName)
                        delete fitem;
                    fitem = nextitem;
                }
                QCString canontargetname = AutoProjectTool::canonicalize(target->name);
                QCString varname = canontargetname + "_SOURCES";
                QStringList sources = QStringList::split(QRegExp("[ \t\n]"), subProject->variables[varname]);
                sources.remove(fileName);
                subProject->variables[varname] = sources.join(" ");
                replaceMap.insert(varname, subProject->variables[varname]);
            }
        }
    }
    
    FileItem *fitem = static_cast<FileItem*>(target->firstChild());
    while (fitem) {
        if (fitem->text(0) == fileName) {
            // Bleh. This leads to a crash later because in a QListView slot items must
            // not be removed
            delete fitem;
            break;
        }
        fitem = static_cast<FileItem*>(fitem->nextSibling());
    }
    QCString canontargetname = AutoProjectTool::canonicalize(target->name);
    QCString varname = canontargetname + "_SOURCES";
    QStringList sources = QStringList::split(QRegExp("[ \t\n]"), subProject->variables[varname]);
    sources.remove(fileName);
    subProject->variables[varname] = sources.join(" ");
    replaceMap.insert(varname, subProject->variables[varname]);
    
    AutoProjectTool::modifyMakefileam(subProject->path + "/Makefile.am", replaceMap);
    
    if (removefromdisk_box->isChecked())
        QFile::remove(subProject->path + "/" + fileName);
        
    m_widget->emitRemovedFile(fileName);
        
    QDialog::accept();
}

#include "removefiledlg.moc"
