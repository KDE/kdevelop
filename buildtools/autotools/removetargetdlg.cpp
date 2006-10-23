/***************************************************************************
                             -------------------
    begin                : 21.11.2002
    copyright            : (C) 2002 by Victor R�er
    email                : victor_roeder@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "removetargetdlg.h"

#include <qapplication.h>
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
#include <klistbox.h>
#include <kprogress.h>
#include <ksqueezedtextlabel.h>
#include <klocale.h>

#include "autolistviewitems.h"

#include "misc.h"

#include "autoprojectpart.h"
#include "autoprojectwidget.h"


RemoveTargetDialog::RemoveTargetDialog( AutoProjectWidget *widget, AutoProjectPart* part, SubprojectItem *spitem,
					TargetItem *titem, QWidget* parent, const char* name )
  : RemoveTargetDlgBase ( parent, name, true, 0 )
{
    removeLabel->setText ( i18n ( "Do you really want to remove <b>%1</b><br>with <b>all files</b> that are attached to it<br>and <b>all dependencies</b>?" ).arg ( titem->name ) );
    directoryLabel->setText ( spitem->path );
    
	if ( titem->name.isEmpty() )
		targetLabel->setText ( i18n ( "%1 in %2" ).arg ( titem->primary ).arg ( titem->prefix ) );
	else
		targetLabel->setText ( titem->name );
    
	connect ( removeButton, SIGNAL ( clicked() ), this, SLOT ( accept() ) );
    connect ( cancelButton, SIGNAL ( clicked() ), this, SLOT ( reject() ) );

    setIcon ( SmallIcon ( "editdelete" ) );

	progressBar->hide();

	m_spitem = spitem;
	m_titem = titem;
	m_widget = widget;
	m_part = part;

	//kdDebug ( 9020 ) << "+++++ " << titem->primary << " " << titem->prefix << " " << titem->name << endl;

	init();
}


RemoveTargetDialog::~RemoveTargetDialog()
{
}

void RemoveTargetDialog::init()
{
	QPtrList <SubprojectItem> subprojectItems = m_widget->allSubprojectItems();

	TargetItem* titem = 0;

	for ( SubprojectItem* spitem = subprojectItems.first(); spitem; spitem = subprojectItems.next() )
	{
		if ( m_titem->name.isEmpty() )
			break;

		for ( titem = spitem->targets.first(); titem; titem = spitem->targets.next() )
		{
			if ( m_titem->name == titem->name )
				continue;

			if ( titem->primary == "LTLIBRARIES" || titem->primary == "PROGRAMS" 
				|| titem->primary == "LIBRARIES"  || titem->primary == "JAVA" )
			{
				QString canonname = AutoProjectTool::canonicalize ( titem->name );

				if ( spitem->variables[canonname + "_LIBADD"].contains ( m_titem->name ) > 0 ||
					spitem->variables[canonname + "_LDADD"].contains ( m_titem->name ) > 0 )
				{
					dependencyListBox->insertItem ( SmallIcon ( "target_kdevelop" ), spitem->path + " (" + titem->name + ")" );

					dependentSubprojects.append ( spitem );
				}
			}
		}
	}

	if ( dependencyListBox->count() == 0 )
		dependencyListBox->insertItem ( i18n("no dependency", "<none>") );
}

void RemoveTargetDialog::accept ()
{
	progressBar->show();
	progressBar->setFormat ( i18n ( "Removing Target... %p%" ) );

	qApp->processEvents();

	QString canonname = AutoProjectTool::canonicalize ( m_titem->name );
	QString varname = m_titem->prefix + "_" + m_titem->primary;

	SubprojectItem* spitem = 0;
	TargetItem* titem = 0;

	QMap <QString, QString> removeMap;
	QMap <QString, QString> replaceMap;

	// Remove dependencies to other targets first (stored by init() in 'dependentTargets')
	for ( spitem = dependentSubprojects.first(); spitem; spitem = dependentSubprojects.next() )
	{
		for ( titem = spitem->targets.first(); titem; titem = spitem->targets.next() )
		{
			QString curVarname;
			QString curCanonname = AutoProjectTool::canonicalize ( titem->name );
			QStringList dependencies;

			if ( spitem->variables[curCanonname + "_LIBADD"].contains ( m_titem->name ) )
				curVarname = curCanonname + "_LIBADD";
			else
				curVarname = curCanonname + "_LDADD";

			dependencies = QStringList::split(QRegExp("[ \t\n]"), spitem->variables[curVarname]);

			//QStringList::Iterator it = dependencies.begin();

			for ( uint i = 0; i < dependencies.size(); ++i )
			{
				QString s = dependencies[i];
				if ( s.contains ( m_titem->name ) > 0 )
					dependencies.remove ( s );
			}

			// if we removed the last entry of "blabla_LDADD" or "blabla_LIBADD", remove the complete line
			if ( dependencies.count() == 0 )
			{
				removeMap.insert ( curVarname, "" );

				AutoProjectTool::removeFromMakefileam ( spitem->path + "/Makefile.am", removeMap );

				removeMap.clear();
			}
			else
			{
				spitem->variables[curVarname] = dependencies.join ( " " );

				replaceMap.insert ( curVarname, spitem->variables[curVarname] );

				AutoProjectTool::modifyMakefileam ( spitem->path + "/Makefile.am", replaceMap );

				replaceMap.clear();
			}
		}
	}

	// handling am_edit stuff
	if ( m_titem->primary == "KDEICON" )
		removeMap.insert ( "KDE_ICON", "" );
	else if ( m_titem->primary == "KDEDOCS" )
		removeMap.insert ( "KDE_DOCS", "" );
	else
	{
		// if we have bin_PROGRAMS = [target to be deleted] [other target]
		// delete only the [target to be deleted], not the whole line!
		QStringList targets = QStringList::split(QRegExp("[ \t\n]"), m_spitem->variables[varname]);
		
		if ( targets.count() > 1 )
		{
			targets.remove ( m_titem->name );
			m_spitem->variables[varname] = targets.join ( " " );
			replaceMap.insert ( varname, m_spitem->variables[varname] );
			AutoProjectTool::modifyMakefileam ( m_spitem->path + "/Makefile.am", replaceMap );
			replaceMap.clear();
		}
		else
		{
			removeMap.insert ( varname, m_titem->name );
		}
	}
		
	// if we have no such line containing blabla_SOURCES, blabla_LDFLAGS, etc.
	// they are ignored
	removeMap.insert ( canonname + "_SOURCES", "" );

	// remove from our internal list
	m_spitem->variables.erase(canonname+"_SOURCES");

	if ( m_titem->primary == "PROGRAMS" || m_titem->primary == "LTLIBRARIES" )
	{
		removeMap.insert ( canonname + "_LDFLAGS", "" );
		removeMap.insert ( canonname + "_DEPENDENCIES", "" );
		removeMap.insert ( canonname + "_LDADD", "" );
		removeMap.insert ( canonname + "_LIBADD", "" );
	}

	AutoProjectTool::removeFromMakefileam ( m_spitem->path + "/Makefile.am", removeMap );

	removeMap.clear();

	// if we have another "blabla_PROGRAMS" or "blabla_LTLIBRARIES" target in the same subproject
	// check if it has an empty "blabla_LIBADD"-entry
	if ( m_titem->primary == "PROGRAMS" || m_titem->primary == "LTLIBRARIES" )
	{
		for ( titem = m_spitem->targets.first(); titem; titem = m_spitem->targets.next() )
		{
			//kdDebug (9020) << "******** " << m_spitem->subdir << endl;
			if ( titem->primary == "PROGRAMS" || titem->primary == "LTLIBRARIES" )
			{
				QString curCanonname = AutoProjectTool::canonicalize ( titem->name );

				if ( m_spitem->variables[curCanonname + "_LIBADD"].isEmpty() )
				{
					removeMap.insert ( curCanonname + "_LIBADD", "" );

					//kdDebug (9020) << "Removing from " << m_spitem->subdir << " " << curCanonname << "_LIBADD" << endl;

					AutoProjectTool::removeFromMakefileam ( m_spitem->path + "/Makefile.am", removeMap );

					removeMap.clear();
				}
			}
		}
	}


	progressBar->setTotalSteps ( m_titem->sources.count() );

	QStringList fileList;

	for ( FileItem* fitem = m_titem->sources.first(); fitem; fitem = m_titem->sources.next() )
	{
		if (removeCheckBox->isChecked())
		{
			// before removing the files, check if they are mentioned in "noinst_HEADERS = blabla1.h blabla2.h"
			QStringList noInstHeaders = QStringList::split ( QRegExp ( "[ \t\n]" ), m_spitem->variables["noinst_HEADERS"] );
			
			if ( noInstHeaders.contains ( fitem->name ) )
			{
				noInstHeaders.remove ( fitem->name );
				
				m_spitem->variables["noinst_HEADERS"] = noInstHeaders.join ( " " );
				replaceMap.insert ( "noinst_HEADERS",  m_spitem->variables["noinst_HEADERS"] );
				AutoProjectTool::modifyMakefileam ( m_spitem->path + "/Makefile.am", replaceMap );
				replaceMap.clear();
			}
			
			QFile::remove(m_spitem->path + "/" + fitem->name);
		}
		
		fileList.append ( m_spitem->path.mid ( m_part->projectDirectory().length() + 1 ) + "/" + fitem->name );

		qApp->processEvents();

		progressBar->setValue ( progressBar->value() + 1 );
	}

	m_widget->emitRemovedFiles ( fileList );

	m_spitem->targets.remove ( m_titem );


	QDialog::accept();
}
