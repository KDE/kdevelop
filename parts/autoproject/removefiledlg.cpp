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

#include "removefiledlg.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>

#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <ksqueezedtextlabel.h>

#include "misc.h"
#include "autoprojectwidget.h"


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
	: RemoveFileDlgBase(parent, name, true)
{
	removeFromTargetsCheckBox = 0;

	QStringList targets;

	QListIterator<TargetItem> it(spitem->targets);
	for (; it.current(); ++it)
		if (fileListContains((*it)->sources, filename))
			targets.append((*it)->name);

	if (targets.count() > 1)
	{
		removeFromTargetsCheckBox = new QCheckBox( fileGroupBox, "removeFromTargetsCheckBox" );
		removeFromTargetsCheckBox->setMinimumSize( QSize( 0, 45 ) );
		fileLayout->addWidget( removeFromTargetsCheckBox );

		QString joinedtargets = "    *" + targets.join("\n    *");
		removeFromTargetsCheckBox->setText ( i18n ( "The file %1 is still used by the following targets:\n%2\n"
												"Remove it from all of them?").arg(filename).arg(joinedtargets) );
	}

	removeLabel->setText ( i18n ( "Do you really want to remove <b>%1</b>?" ).arg ( filename ) );
	directoryLabel->setText ( spitem->path );
	targetLabel->setText ( item->name );

	connect ( removeButton, SIGNAL ( clicked() ), this, SLOT ( accept() ) );
	connect ( cancelButton, SIGNAL ( clicked() ), this, SLOT ( reject() ) );

	setIcon ( SmallIcon ( "editdelete.png" ) );

	m_widget = widget;
	subProject = spitem;
	target = item;
	fileName = filename;
}


RemoveFileDialog::~RemoveFileDialog()
{}


void RemoveFileDialog::accept()
{
	QMap<QString,QString> replaceMap;

	if (removeFromTargetsCheckBox && removeFromTargetsCheckBox->isChecked()) {
		QListIterator<TargetItem> it(subProject->targets);
		for (; it.current(); ++it) {
			if ((*it) != target && fileListContains((*it)->sources, fileName)) {
				FileItem *fitem = static_cast<FileItem*>((*it)->firstChild());
				while (fitem) {
					FileItem *nextitem = static_cast<FileItem*>(fitem->nextSibling());
					if (fitem->text(0) == fileName) {
						QListView *lv = fitem->listView();
						lv->setSelected(fitem, false);
						(*it)->sources.remove(fitem);
					}
					fitem = nextitem;
				}
				QString canontargetname = AutoProjectTool::canonicalize((*it)->name);
				QString varname = canontargetname + "_SOURCES";
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
			QListView *lv = fitem->listView();
			lv->setSelected(fitem, false);
			target->sources.remove(fitem);
			break;
		}
		fitem = static_cast<FileItem*>(fitem->nextSibling());
	}
	QString canontargetname = AutoProjectTool::canonicalize(target->name);
	QString varname = canontargetname + "_SOURCES";
	QStringList sources = QStringList::split(QRegExp("[ \t\n]"), subProject->variables[varname]);
	sources.remove(fileName);
	subProject->variables[varname] = sources.join(" ");
	replaceMap.insert(varname, subProject->variables[varname]);

	AutoProjectTool::modifyMakefileam(subProject->path + "/Makefile.am", replaceMap);

	if (removeCheckBox->isChecked())
		QFile::remove(subProject->path + "/" + fileName);

	kdDebug ( 9000 ) << "Removing file: " << subProject->path.mid ( m_widget->projectDirectory().length() + 1 ) + "/" + fileName << endl;
		
	m_widget->emitRemovedFile ( subProject->path.mid ( m_widget->projectDirectory().length() + 1 ) + "/" + fileName );

	QDialog::accept();
}

#include "removefiledlg.moc"
