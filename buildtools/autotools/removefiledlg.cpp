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
#include <q3groupbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>

#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <ksqueezedtextlabel.h>

#include "autolistviewitems.h"

#include "misc.h"
#include "autoprojectpart.h"
#include "autoprojectwidget.h"
#include "autodetailsview.h"

static bool fileListContains(const Q3PtrList<FileItem> &list, const QString &name)
{
	Q3PtrListIterator<FileItem> it(list);
	for (; it.current(); ++it)
		if ((*it)->text(0) == name)
			return true;
	return false;
}


RemoveFileDialog::RemoveFileDialog(AutoProjectWidget *widget, AutoProjectPart* part, SubprojectItem *spitem,
								TargetItem *item, const QString &filename,
								QWidget *parent, const char *name)
	: RemoveFileDlgBase(parent, name, true)
{
	removeFromTargetsCheckBox = 0;

	QStringList targets;

	Q3PtrListIterator<TargetItem> it(spitem->targets);
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
        setMinimumSize(QSize(size().width(), size().height() + removeFromTargetsCheckBox->size().height()*2) );
	}

	removeLabel->setText ( i18n ( "Do you really want to remove <b>%1</b>?" ).arg ( filename ) );

	directoryLabel->setText ( spitem->path );
	if ( item->name.isEmpty() )
		targetLabel->setText ( i18n ( "%1 in %2" ).arg ( item->primary ).arg ( item->prefix ) );
	else
		targetLabel->setText ( item->name );

	connect ( removeButton, SIGNAL ( clicked() ), this, SLOT ( accept() ) );
	connect ( cancelButton, SIGNAL ( clicked() ), this, SLOT ( reject() ) );

	setIcon ( SmallIcon ( "editdelete.png" ) );

	m_widget = widget;
	m_part = part;
	subProject = spitem;
	target = item;
	fileName = filename;
}


RemoveFileDialog::~RemoveFileDialog()
{}


void RemoveFileDialog::accept()
{
	m_widget->emitRemovedFile ( subProject->path.mid ( m_part->projectDirectory().length() + 1 ) + "/" + fileName );

	QMap<QString,QString> replaceMap;

	if (removeFromTargetsCheckBox && removeFromTargetsCheckBox->isChecked()) {
		Q3PtrListIterator<TargetItem> it(subProject->targets);
		for (; it.current(); ++it) {
			if ((*it) != target && fileListContains((*it)->sources, fileName)) {
				FileItem *fitem = static_cast<FileItem*>((*it)->firstChild());
				while (fitem) {
					FileItem *nextitem = static_cast<FileItem*>(fitem->nextSibling());
					if (fitem->text(0) == fileName) {
						Q3ListView *lv = fitem->listView();
						lv->setSelected(fitem, false);
						(*it)->sources.remove(fitem);
					}
					fitem = nextitem;
				}
				QString canontargetname = AutoProjectTool::canonicalize((*it)->name);
                                QString varname;
                                if( (*it)->primary == "PROGRAMS" || (*it)->primary == "LIBRARIES" || (*it)->primary == "LTLIBRARIES" )
                                    varname = canontargetname + "_SOURCES";
                                else
                                    varname = (*it)->prefix + "_" + (*it)->primary;
				QStringList sources = QStringList::split(QRegExp("[ \t\n]"), subProject->variables[varname]);
				sources.remove(fileName);
				subProject->variables[varname] = sources.join(" ");
				replaceMap.insert(varname, subProject->variables[varname]);
			}
		}
	}

	QString fileItemName;
	FileItem *fitem = static_cast<FileItem*>(target->firstChild());
	while (fitem) {
		if (fitem->text(0) == fileName) {
			Q3ListView *lv = fitem->listView();
			lv->setSelected(fitem, false);
			fileItemName = fitem->name;
			target->sources.remove(fitem);
			break;
		}
		fitem = static_cast<FileItem*>(fitem->nextSibling());
	}
	QString canontargetname = AutoProjectTool::canonicalize(target->name);
	QString varname;
	if( target->primary == "PROGRAMS" || target->primary == "LIBRARIES" || target->primary == "LTLIBRARIES" )
	    varname = canontargetname + "_SOURCES";
	else
	    varname = target->prefix + "_" + target->primary;
	QStringList sources = QStringList::split(QRegExp("[ \t\n]"), subProject->variables[varname]);
	sources.remove(fileName);
	subProject->variables[varname] = sources.join(" ");
	replaceMap.insert(varname, subProject->variables[varname]);

	AutoProjectTool::modifyMakefileam(subProject->path + "/Makefile.am", replaceMap);

//  review configuration cleanup in the project file after removing subclassing related source
	QDomDocument &dom = *(m_part->projectDom());

	QDomElement el = dom.documentElement();
	QDomNode el2 = el.namedItem("kdevautoproject");
	QDomNode el3 = el2.namedItem("subclassing");

	QDomNode n = el3.firstChild();
	Q3ValueList<QDomNode> nodesToRemove;
	while ( !n.isNull() ) {
		QDomNamedNodeMap attr = n.attributes();
		QString fpath = subProject->path + QString("/") + fileItemName;
		QString relpath = fpath.remove(0, m_part->projectDirectory().length());
		if ((attr.item(0).nodeValue() == relpath)
			|| (attr.item(1).nodeValue() == relpath) )
			nodesToRemove.append(n);
		n = n.nextSibling();
	}
	Q3ValueList<QDomNode>::iterator it;
	for ( it = nodesToRemove.begin(); it != nodesToRemove.end(); ++it )
		el3.removeChild(*it);

	if (removeCheckBox->isChecked())
		QFile::remove(subProject->path + "/" + fileName);

	QDialog::accept();
}

#include "removefiledlg.moc"
