/***************************************************************************
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qhbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qapplication.h>
#include <kdebug.h>
#include <kseparator.h>
#include <klocale.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kmessagebox.h>

#include "kdevcompileroptions.h"
#include "projectoptionsdlg.h"


GeneralPage::GeneralPage(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setAutoAdd(true);
    
    new QLabel("Meep!", this);
    
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addStretch();
}


GeneralPage::~GeneralPage()
{}


ProjectOptionsDialog::ProjectOptionsDialog(QWidget *parent, const char *name)
    : KDialogBase(TreeList, i18n("Project Options"), Ok|Cancel, Ok,
                  parent, name)
{
    QVBox *vbox;

    vbox = addVBoxPage(i18n("General"));
    generalpage = new GeneralPage(vbox, "general page");

    vbox = addVBoxPage(i18n("Compilers"));
    compilerpage = new CompilerPage(vbox, "compiler page");
}


ProjectOptionsDialog::~ProjectOptionsDialog()
{}


void ProjectOptionsDialog::accept()
{
    kdDebug(9000) << "CFLAGS=" << compilerpage->cFlags() << endl;
    kdDebug(9000) << "CXXFLAGS=" << compilerpage->cxxFlags() << endl;
    KDialogBase::accept();
}

#include "projectoptionsdlg.moc"
