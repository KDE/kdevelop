/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "addfiledlg.h"

#include <qcheckbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtextstream.h>

#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kurl.h>

#include "autolistviewitems.h"

#include "filetemplate.h"
#include "misc.h"
#include "urlutil.h"
#include "autoprojectpart.h"
#include "autoprojectwidget.h"

#include "kdevpartcontroller.h"

AddFileDialog::AddFileDialog(AutoProjectPart *part, AutoProjectWidget *widget,
							SubprojectItem *spitem, TargetItem *item,
							QWidget *parent, const char *name)
	: AddFileDlgBase(parent, name, true)
{
	connect ( createButton, SIGNAL ( clicked() ), this, SLOT ( accept() ) );
	connect ( cancelButton, SIGNAL ( clicked() ), this, SLOT ( reject() ) );

	directoryLabel->setText ( spitem->path );
	if ( item->name.isEmpty() )
		targetLabel->setText ( i18n ( "%1 in %2" ).arg ( item->primary ).arg ( item->prefix ) );
	else
		targetLabel->setText ( item->name );

	setIcon ( SmallIcon ( "filenew.png" ) );

	m_part = part;
	m_widget = widget;
	subProject = spitem;
	target = item;
}


AddFileDialog::~AddFileDialog()
{}


void AddFileDialog::accept()
{
	QString name = fileEdit->text();
	if (name.find('/') != -1) {
		KMessageBox::sorry(this, i18n("Please enter the file name without '/' and so on."));
		return;
	}

	QListViewItem *child = target->firstChild();
	while (child) {
		FileItem *item = static_cast<FileItem*>(child);
		if (name == item->name) {
			KMessageBox::sorry(this, i18n("This file is already in the target."));
			return;
		}
		child = child->nextSibling();
	}

	if (templateCheckBox->isChecked()) {
		QString srcdir = m_part->projectDirectory();
		QString destdir = subProject->path;
		QString destpath = destdir + "/" + name;
		if (QFileInfo(destpath).exists()) {
			KMessageBox::sorry(this, i18n("<b>A file with this name already exists.</b><br><br>Please use the \"Add existing file\" dialog."));
			return;
		}
		if( !FileTemplate::copy(m_part, QFileInfo(name).extension(), destpath) )
		    kdDebug(9020) << "cannot create file " << destpath << endl;
	} else {
		// create an empty file
		QString srcdir = m_part->projectDirectory();
		QString destdir = subProject->path;
		QString destpath = destdir + "/" + name;

		if (QFileInfo(destpath).exists()) {
			KMessageBox::sorry(this, i18n("<b>A file with this name already exists.</b><br><br>Please use the \"Add existing file\" dialog."));
			return;
		}

		QFile f( destpath );
		if( f.open(IO_WriteOnly) )
		    f.close();
	}

	FileItem *fitem = m_widget->createFileItem(name, subProject);
	target->sources.append(fitem);
	target->insertItem(fitem);

	QString canontargetname = AutoProjectTool::canonicalize(target->name);
	QString varname;
        if( target->primary == "PROGRAMS" || target->primary == "LIBRARIES" || target->primary == "LTLIBRARIES" )
            varname = canontargetname + "_SOURCES";
        else
            varname = target->prefix + "_" + target->primary;
	subProject->variables[varname] += (" " + name);

	QMap<QString,QString> replaceMap;
	replaceMap.insert(varname, subProject->variables[varname]);

	AutoProjectTool::modifyMakefileam(subProject->path + "/Makefile.am", replaceMap);

	m_widget->emitAddedFile( subProject->path.mid ( m_part->project()->projectDirectory().length() + 1 ) + "/" + name );
	m_part->partController()->editDocument ( KURL ( subProject->path + "/" + name ) );

	QDialog::accept();
}

#include "addfiledlg.moc"
