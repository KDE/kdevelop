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

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qstrlist.h>
#include <kbuttonbox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "misc.h"
#include "autoprojectwidget.h"
#include "addtargetdlg.h"


AddTargetDialog::AddTargetDialog(AutoProjectWidget *widget, SubprojectItem *item,
                                 QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Add target"));

    subProject = item;
    m_widget = widget;

    QLabel *primary_label = new QLabel(i18n("Primary:"), this);
    primary_combo = new QComboBox(this);
    primary_combo->setFocus();
    primary_combo->insertItem(i18n("Program"));
    primary_combo->insertItem(i18n("Library"));
    primary_combo->insertItem(i18n("Libtool library"));
    primary_combo->insertItem(i18n("Script"));
    primary_combo->insertItem(i18n("Header"));
    primary_combo->insertItem(i18n("Data file"));
    primary_combo->insertItem(i18n("Java"));
    connect( primary_combo, SIGNAL(activated(int)), this, SLOT(primaryChanged()) );

    QLabel *prefix_label = new QLabel(i18n("Prefix:"), this);
    prefix_combo = new QComboBox(this);
    primaryChanged(); // updates the combo

    QLabel *name_label = new QLabel(i18n("Name:"), this);
    name_edit = new QLineEdit(this);
    
    QVBoxLayout *layout = new QVBoxLayout(this, 2*KDialog::marginHint(), KDialog::spacingHint());
    
    QGridLayout *grid = new QGridLayout(3, 2);
    layout->addLayout(grid);
    grid->addWidget(primary_label, 0, 0);
    grid->addWidget(primary_combo, 0, 1);
    grid->addWidget(prefix_label, 1, 0);
    grid->addWidget(prefix_combo, 1, 1);
    grid->addWidget(name_label, 2, 0);
    grid->addWidget(name_edit, 2, 1);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok_button = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel_button = buttonbox->addButton(i18n("Cancel"));
    ok_button->setDefault(true);
    connect( ok_button, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel_button, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);
}


AddTargetDialog::~AddTargetDialog()
{}


void AddTargetDialog::accept()
{
    QCString name = name_edit->text().stripWhiteSpace().latin1();
    QCString prefix = prefix_combo->currentText().latin1();

    QCString primary;
    switch (primary_combo->currentItem()) {
    case 0: primary = "PROGRAMS";    break;
    case 1: primary = "LIBRARIES";   break;
    case 2: primary = "LTLIBRARIES"; break;
    case 3: primary = "SCRIPTS";     break;
    case 4: primary = "HEADERS";     break;
    case 5: primary = "DATA";        break;
    case 6: primary = "JAVA";        break;
    default: ;
    }
    
    if (name.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to give the target a name"));
        return;
    }

    QListIterator<TargetItem> it(subProject->targets);
    for (; it.current(); ++it)
        if (name == (*it)->name) {
            KMessageBox::sorry(this, i18n("A target with this name already exists"));
            return;
        }

    TargetItem *titem = m_widget->createTargetItem(name, prefix, primary);
    subProject->targets.append(titem);
    
    QCString canonname = AutoProjectTool::canonicalize(name);
    QCString varname = prefix + "_" + primary;
    subProject->variables[varname] += (QCString(" ") + name);
    
    QMap<QCString,QCString> replaceMap;
    replaceMap.insert(varname, subProject->variables[varname]);
    replaceMap.insert(canonname + "_SOURCES", "");

    AutoProjectTool::modifyMakefileam(subProject->path + "/Makefile.am", replaceMap);
    
    QDialog::accept();
}


void AddTargetDialog::primaryChanged()
{
    QStrList list;
    switch (primary_combo->currentItem()) {
    case 0: // Program
        list.append("bin");
        list.append("sbin");
        list.append("libexec");
        list.append("pkglib");
        list.append("noinst");
        break;
    case 1: // Library
    case 2: // Libtool library
        list.append("lib");
        list.append("pkglib");
        list.append("noinst");
        break;
    case 3: // Script
        list.append("bin");
        list.append("sbin");
        list.append("libexec");
        list.append("pkgdata");
        list.append("noinst");
        break;
    case 4: // Header
        list.append("include");
        list.append("oldinclude");
        list.append("pkginclude");
        break;
    case 5: // Data
        list.append("bin");
        list.append("sbin");
        break;
    case 6: // Java
        list.append("java");
        break;
    }

    prefix_combo->clear();
    
    prefix_combo->insertStrList(list);
    QStrList prefixes;
    QMap<QCString,QCString>::ConstIterator it;
    for (it = subProject->prefixes.begin(); it != subProject->prefixes.end(); ++it)
        prefix_combo->insertItem(QString(it.key()));
}

#include "addtargetdlg.moc"
