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

#include "addtargetdlg.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qvalidator.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>

#include "autolistviewitems.h"

#include "misc.h"
#include "autodetailsview.h"
#include "autoprojectwidget.h"


AddTargetDialog::AddTargetDialog(AutoProjectWidget *widget, SubprojectItem *item,
								QWidget *parent, const char *name)
	: AddTargetDialogBase(parent, name, true)
{
	m_subproject = item;
	m_widget = widget;
// 	m_detailsView = view;

	primary_combo->setFocus();
	primary_combo->insertItem(i18n("Program"));
	primary_combo->insertItem(i18n("Library"));
	primary_combo->insertItem(i18n("Libtool Library"));
	primary_combo->insertItem(i18n("Script"));
	primary_combo->insertItem(i18n("Header"));
	primary_combo->insertItem(i18n("Data File"));
	primary_combo->insertItem(i18n("Java"));

	primaryChanged(); // updates prefix combo

	if (widget->kdeMode())
		ldflagsother_edit->setText("$(all_libraries)");

	connect( filename_edit, SIGNAL( textChanged(const QString&) ), this, SLOT( slotFileNameChanged (const QString&) ) );

	setIcon ( SmallIcon ( "targetnew_kdevelop.png" ) );

	canonicalLabel->setText ( QString::null );
}


AddTargetDialog::~AddTargetDialog()
{}


void AddTargetDialog::primaryChanged()
{
	QStringList list;
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
		if (m_widget->kdeMode())
			list.append("kde_module");
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
		list.append("noinst");
		break;
	case 5: // Data
		list.append("bin");
		list.append("sbin");
		list.append("noinst");
		break;
	case 6: // Java
		list.append("java");
		list.append("noinst");
		break;
	}

	prefix_combo->clear();

	prefix_combo->insertStringList(list);
	QStringList prefixes;
	QMap<QString,QString>::ConstIterator it;
	for (it = m_subproject->prefixes.begin(); it != m_subproject->prefixes.end(); ++it)
		prefix_combo->insertItem(it.key());

	// Only enable ldflags stuff for libtool libraries
	bool lt = primary_combo->currentItem() == 2;
	bool prog = primary_combo->currentItem() == 0;
	allstatic_box->setEnabled(lt);
	avoidversion_box->setEnabled(lt);
	module_box->setEnabled(lt);
	noundefined_box->setEnabled(lt);
	ldflagsother_edit->setEnabled(lt || prog);
}


void AddTargetDialog::accept()
{
	QString name = filename_edit->text().stripWhiteSpace();
	QString prefix = prefix_combo->currentText();

	QString primary;
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

	if (primary == "DATA"){
	    // DATA does not need a name; DATA may already exist.
	    TargetItem *titem = m_widget->createTargetItem(name, prefix, primary, true);
	    QPtrListIterator<TargetItem> it( m_subproject->targets );
	    for( ; it.current(); ++it ){
		if( (*it)->text(0) == titem->text(0) ){
		    /// \FIXME Add message box here, after string-freeze is over
		    ///        something like: "This data target already exists."
		    QDialog::accept();
		    return;
		}
	    }
	    m_subproject->targets.append( titem );
	    QDialog::accept();
	    return;
	}

	if (name.isEmpty()) {
		KMessageBox::sorry(this, i18n("You have to give the target a name"));
		return;
	}

#if 0
	if (primary == "LIBRARIES" && !name.startsWith("lib")) {
		KMessageBox::sorry(this, i18n("Libraries must have a lib prefix."));
		return;
	}

	if (primary == "LTLIBRARIES" && !name.startsWith("lib")) {
		KMessageBox::sorry(this, i18n("Libtool libraries must have a lib prefix."));
		return;
	}

	if (primary == "LTLIBRARIES" && name.right(3) != ".la") {
		KMessageBox::sorry(this, i18n("Libtool libraries must have a .la suffix."));
		return;
	}

#endif

	if( primary.endsWith("LIBRARIES") && !name.startsWith("lib") )
	    name.prepend( QString::fromLatin1("lib") );

	if( primary == "LTLIBRARIES" && !name.endsWith(".la") )
	    name.append( QString::fromLatin1(".la") );

	if ( primary == "LIBRARIES" && !name.endsWith(".a") )
	    name.append ( QString::fromLatin1(".a") );

	QPtrListIterator<TargetItem> it(m_subproject->targets);
	for (; it.current(); ++it)
		if (name == (*it)->name) {
			KMessageBox::sorry(this, i18n("A target with this name already exists."));
			return;
		}

	QStringList flagslist;
	if (primary == "LTLIBRARIES") {
		if (allstatic_box->isChecked())
			flagslist.append("-all-static");
		if (avoidversion_box->isChecked())
			flagslist.append("-avoid-version");
		if (module_box->isChecked())
			flagslist.append("-module");
		if (noundefined_box->isChecked())
			flagslist.append("-no-undefined");
	}
	flagslist.append(ldflagsother_edit->text());
	QString ldflags = flagslist.join( " " );

	TargetItem *titem = m_widget->createTargetItem(name, prefix, primary, false);
	// m_detailsView->insertItem ( titem );
	m_subproject->targets.append(titem);

	QString canonname = AutoProjectTool::canonicalize(name);

	QMap<QString,QString> replaceMap;

        if( primary == "PROGRAMS" || primary == "LIBRARIES" || primary == "LTLIBRARIES" ){
	    QString varname = prefix + "_" + primary;
	    m_subproject->variables[varname] += (" " + name);
	    replaceMap.insert(varname, m_subproject->variables[varname]);
            replaceMap.insert(canonname + "_SOURCES", "");
        }
	if (primary == "LTLIBRARIES" || primary == "PROGRAMS")
            replaceMap.insert(canonname + "_LDFLAGS", ldflags);

	AutoProjectTool::modifyMakefileam(m_subproject->path + "/Makefile.am", replaceMap);

	QDialog::accept();
}

void AddTargetDialog::slotFileNameChanged ( const QString& text )
{
	canonicalLabel->setText ( AutoProjectTool::canonicalize ( text ) );
}

#include "addtargetdlg.moc"
