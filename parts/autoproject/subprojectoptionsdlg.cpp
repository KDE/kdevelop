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

#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <keditlistbox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kservice.h>

#include "domutil.h"
#include "misc.h"
#include "addprefixdlg.h"
#include "autoprojectpart.h"
#include "autoprojectwidget.h"
#include "subprojectoptionsdlg.h"


SubprojectOptionsDialog::SubprojectOptionsDialog(AutoProjectPart *part, AutoProjectWidget *widget,
                                                 SubprojectItem *item, QWidget *parent, const char *name)
    : QTabDialog(parent, name, true)
{
    setCaption(i18n("Subproject options for '%1'").arg(item->subdir));

    subProject = item;
    m_widget = widget;
    m_part = part;

    addTab(createCompilerTab(), i18n("Compiler"));
    addTab(createIncludeTab(), i18n("Includes"));
    addTab(createPrefixTab(), i18n("Prefixes"));
    
    setCancelButton();
    connect( this, SIGNAL(applyButtonPressed()), this, SLOT(accept()) );
    connect( this, SIGNAL(cancelButtonPressed()), this, SLOT(reject()) );

    init();
}


QWidget *SubprojectOptionsDialog::createCompilerTab()
{
    QWidget *w = new QWidget(this);
    
    QLabel *cflags_label      = new QLabel(i18n("Compiler flags for C Compiler (CFLAGS):"), w);
    QLabel *cxxflags_label    = new QLabel(i18n("Compiler flags for C++ Compiler (CXXFLAGS):"), w);
    QLabel *f77flags_label    = new QLabel(i18n("Compiler flags for Fortran Compiler (FFLAGS):"), w);
    
    QHBox *cflags_box   = new QHBox(w);
    cflags_box->setSpacing(4);
    QHBox *cxxflags_box = new QHBox(w);
    cxxflags_box->setSpacing(4);
    QHBox *f77flags_box = new QHBox(w);
    f77flags_box->setSpacing(4);
    
    cflags_edit   = new QLineEdit(cflags_box);
    cxxflags_edit = new QLineEdit(cxxflags_box);
    f77flags_edit = new QLineEdit(f77flags_box);

    QFontMetrics fm(cflags_edit->fontMetrics());
    int wid = fm.width('X')*35;
    cflags_edit->setMinimumWidth(wid);
    cxxflags_edit->setMinimumWidth(wid);
    f77flags_edit->setMinimumWidth(wid);
    
    QPushButton *cflags_button   = new QPushButton("...", cflags_box);
    cflags_button->setFixedSize(30, 25);
    QPushButton *cxxflags_button = new QPushButton("...", cxxflags_box);
    cxxflags_button->setFixedSize(30, 25);
    QPushButton *f77flags_button = new QPushButton("...", f77flags_box);
    f77flags_button->setFixedSize(30, 25);

    QDomDocument &doc = *m_part->document();
    //    kdDebug(9020) << "ccompiler: " << DomUtil::readEntry(doc, "/kdevautoproject/compiler/ccompiler") << endl;
    //    kdDebug(9020) << "hasservice: " << (bool)KService::serviceByName(DomUtil::readEntry(doc, "/kdevautoproject/compiler/ccompiler")) << endl;

    if (!KService::serviceByName(DomUtil::readEntry(doc, "/kdevautoproject/compiler/ccompiler")))
        cflags_button->setEnabled(false);
    if (!KService::serviceByName(DomUtil::readEntry(doc, "/kdevautoproject/compiler/cxxcompiler")))
        cxxflags_button->setEnabled(false);
    if (!KService::serviceByName(DomUtil::readEntry(doc, "/kdevautoproject/compiler/f77compiler")))
        f77flags_button->setEnabled(false);
                            
    connect(cflags_button,   SIGNAL(clicked()), this, SLOT(cflagsClicked()));
    connect(cxxflags_button, SIGNAL(clicked()), this, SLOT(cxxflagsClicked()));
    connect(f77flags_button, SIGNAL(clicked()), this, SLOT(f77flagsClicked()));

    QBoxLayout *layout = new QVBoxLayout(w, 2*KDialog::marginHint(), KDialog::spacingHint());
    layout->addWidget(cflags_label);
    layout->addWidget(cflags_box);
    layout->addSpacing(6);
    layout->addWidget(cxxflags_label);
    layout->addWidget(cxxflags_box);
    layout->addSpacing(6);
    layout->addWidget(f77flags_label);
    layout->addWidget(f77flags_box);

    return w;
}


QWidget *SubprojectOptionsDialog::createIncludeTab()
{
    QWidget *w = new QWidget(this);

    //    QLabel *include_label = new QLabel(i18n("Include directories:"), w);
    
    include_view = new KEditListBox(i18n("Include directories"), w);

    QBoxLayout *layout = new QVBoxLayout(w, 2*KDialog::marginHint(), KDialog::spacingHint());
    //    layout->addWidget(include_label);
    layout->addWidget(include_view);

    return w;
}


QWidget *SubprojectOptionsDialog::createPrefixTab()
{
    QWidget *w = new QWidget(this);

    QLabel *prefix_label = new QLabel(i18n("Custom prefixes:"), w);
    
    prefix_view = new QListView(w);
    prefix_view->setAllColumnsShowFocus(true);
    prefix_view->addColumn(i18n("Name"));
    prefix_view->addColumn(i18n("Path"));
    
    QVBox *buttonbox = new QVBox(w);
    buttonbox->setMargin(KDialog::spacingHint());
    connect( new QPushButton(i18n("Add..."), buttonbox), SIGNAL(clicked()), this, SLOT(addPrefixClicked()) );
    connect( new QPushButton(i18n("Remove"), buttonbox), SIGNAL(clicked()), this, SLOT(removePrefixClicked()) );

    QGridLayout *grid = new QGridLayout(w, 2, 2, 2*KDialog::marginHint(), KDialog::spacingHint());
    grid->addMultiCellWidget(prefix_label, 0, 0, 0, 1);
    grid->addWidget(prefix_view, 1, 0);
    grid->addWidget(buttonbox, 1, 1);
    
    return w;
}


SubprojectOptionsDialog::~SubprojectOptionsDialog()
{}


void SubprojectOptionsDialog::init()
{

    cflags_edit->setText(subProject->variables["AM_CFLAGS"]);
    cxxflags_edit->setText(subProject->variables["AM_CXXFLAGS"]);
    f77flags_edit->setText(subProject->variables["AM_FFLAGS"]);

    QCString includes = subProject->variables["INCLUDES"];
    QStringList l = QStringList::split(QRegExp("[ \t]"), QString(includes));
    include_view->insertStringList(l);
    
    //    QStringList::ConstIterator it1;
    //    for (it1 = l.begin(); it1 != l.end(); ++it1)
    //        include_view->in
    
    QMap<QCString, QCString>::ConstIterator it2;
    for (it2 = subProject->prefixes.begin(); it2 != subProject->prefixes.end(); ++it2)
        new QListViewItem(prefix_view, it2.key(), it2.data());
}


void SubprojectOptionsDialog::accept()
{
    QMap<QCString, QCString> replaceMap;
    
    QCString old_cflags = subProject->variables["AM_CFLAGS"];
    QCString new_cflags = cflags_edit->text().latin1();
    if (new_cflags != old_cflags) {
        subProject->variables["AM_CFLAGS"] = new_cflags;
        replaceMap.insert(QCString("AM_CFLAGS"), new_cflags);
    }

    QCString old_cxxflags = subProject->variables["AM_CXXFLAGS"];
    QCString new_cxxflags = cxxflags_edit->text().latin1();
    if (new_cxxflags != old_cxxflags) {
        subProject->variables["AM_CXXFLAGS"] = new_cxxflags;
        replaceMap.insert(QCString("AM_CXXFLAGS"), new_cxxflags);
    }

    QCString old_fflags = subProject->variables["AM_FFLAGS"];
    QCString new_fflags = f77flags_edit->text().latin1();
    if (new_fflags != old_fflags) {
        subProject->variables["AM_FFLAGS"] = new_fflags;
        replaceMap.insert(QCString("AM_FFLAGS"), new_fflags);
    }

    QCString includes = include_view->items().join(" ").latin1();
    subProject->variables["INCLUDES"] = includes;
    replaceMap.insert("INCLUDES", includes);
    
    for (QListViewItem *item = prefix_view->firstChild();
         item; item = item->nextSibling())
        replaceMap.insert(item->text(0).latin1(), item->text(1).latin1());

    // FIXME: take removed items into account
    
    AutoProjectTool::modifyMakefileam(m_widget->subprojectDirectory() + "/Makefile.am", replaceMap);

    QTabDialog::accept();
}


void SubprojectOptionsDialog::cflagsClicked()
{
    QString ccompiler = DomUtil::readEntry(*m_part->document(), "/kdevautoproject/compiler/ccompiler");
    QString new_cflags = AutoProjectTool::execFlagsDialog(ccompiler, cflags_edit->text(), this);
    if (!new_cflags.isNull())
        cflags_edit->setText(new_cflags);
}


void SubprojectOptionsDialog::cxxflagsClicked()
{
    QString cxxcompiler = DomUtil::readEntry(*m_part->document(), "/kdevautoproject/compiler/cxxcompiler");
    QString new_cxxflags = AutoProjectTool::execFlagsDialog(cxxcompiler, cxxflags_edit->text(), this);
    if (!new_cxxflags.isNull())
        cxxflags_edit->setText(new_cxxflags);
}


void SubprojectOptionsDialog::f77flagsClicked()
{
    QString f77compiler = DomUtil::readEntry(*m_part->document(), "/kdevautoproject/compiler/f77compiler");
    QString new_fflags = AutoProjectTool::execFlagsDialog(f77compiler, f77flags_edit->text(), this);
    if (!new_fflags.isNull())
        f77flags_edit->setText(new_fflags);
}


void SubprojectOptionsDialog::addPrefixClicked()
{
    AddPrefixDialog dlg;
    if (!dlg.exec())
        return;
    
    new QListViewItem(prefix_view, dlg.name(), dlg.path());
}


void SubprojectOptionsDialog::removePrefixClicked()
{
    delete prefix_view->currentItem();
}

#include "subprojectoptionsdlg.moc"
