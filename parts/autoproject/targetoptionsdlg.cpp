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

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <kbuttonbox.h>
#include <kdialog.h>
#include <klocale.h>

#include "misc.h"
#include "autoprojectwidget.h"
#include "targetoptionsdlg.h"


TargetOptionsDialog::TargetOptionsDialog(AutoProjectWidget *widget, TargetItem *item,
                                         QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption( i18n("Target options for '%1'").arg(item->name) );

    target = item;
    m_widget = widget;

    QString addvarname =
        (item->primary == "PROGRAMS") || (item->primary == "LTLIBRARIES")? "LDADD" : "LIBADD";
    QLabel *ldlibadd_label = new QLabel(i18n("Libraries to link with (%1)").arg(addvarname), this);
    ldlibadd_edit = new QLineEdit(this);
    ldlibadd_edit->setFocus();
    QFontMetrics fm(ldlibadd_edit->fontMetrics());
    ldlibadd_edit->setMinimumWidth(fm.width('X')*35);

    QLabel *ldflags_label = new QLabel(i18n("Linker flags (LDFLAGS)"), this);
    ldflags_edit = new QLineEdit(this);

    QLabel *dependencies_label = new QLabel(i18n("Dependencies (DEPENDENCIES)"), this);
    dependencies_edit = new QLineEdit(this);

    QBoxLayout *layout = new QVBoxLayout(this, 2*KDialog::marginHint(), KDialog::spacingHint());

    QGridLayout *grid = new QGridLayout(3, 2);
    layout->addLayout(grid);
    grid->addWidget(ldlibadd_label, 0, 0);
    grid->addWidget(ldlibadd_edit, 0, 1);
    grid->addWidget(ldflags_label, 1, 0);
    grid->addWidget(ldflags_edit, 1, 1);
    grid->addWidget(dependencies_label, 2, 0);
    grid->addWidget(dependencies_edit, 2, 1);

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
    ok_button->setDefault(true);
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    init();
}


TargetOptionsDialog::~TargetOptionsDialog()
{}


void TargetOptionsDialog::init()
{
    if (target->primary == "PROGRAMS" || target->primary == "LTLIBRARIES")
        ldlibadd_edit->setText(target->ldadd);
    else
        ldlibadd_edit->setText(target->libadd);
    ldflags_edit->setText(target->ldflags);
    dependencies_edit->setText(target->dependencies);
}


void TargetOptionsDialog::accept()
{
    QMap<QCString, QCString> replaceMap;
    
    if (target->primary == "PROGRAMS" || target->primary == "LTLIBRARIES") {
        QCString old_ldadd = target->ldadd;
        QCString new_ldadd = ldlibadd_edit->text().latin1();
        if (new_ldadd != old_ldadd) {
            target->ldadd = new_ldadd;
            replaceMap.insert(target->name + "_LDADD", new_ldadd);
        }
    }
    
    if (target->primary == "LIBRARIES") {
        QCString old_libadd = target->libadd;
        QCString new_libadd = ldlibadd_edit->text().latin1();
        if (new_libadd != old_libadd) {
            target->libadd = new_libadd;
            replaceMap.insert(target->name + "_LIBADD", new_libadd);
        }
    }
    
    QCString old_ldflags = target->ldflags;
    QCString new_ldflags = ldflags_edit->text().latin1();
    if (new_ldflags != old_ldflags) {
        target->ldflags = new_ldflags;
        replaceMap.insert(target->name + "_LDFLAGS", new_ldflags);
    }

    QCString old_dependencies = target->dependencies;
    QCString new_dependencies = dependencies_edit->text().latin1();
    if (new_dependencies != old_dependencies) {
        target->dependencies = new_dependencies;
        replaceMap.insert(target->name + "_DEPENDENCIES", new_dependencies);
    }

    // We can safely assume that this target is in the active sub project
    AutoProjectTool::modifyMakefileam(m_widget->subprojectDirectory() + "/Makefile.am", replaceMap);
    
    QDialog::accept();
}

#include "targetoptionsdlg.moc"
