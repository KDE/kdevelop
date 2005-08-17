/***************************************************************************
*   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   Copyright (C) 2002 by Victor Rder                                    *
*   victor_roeder@gmx.de                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qpainter.h>
#include <qinputdialog.h>
#include <qregexp.h>

#include <kiconloader.h>
#include "misc.h"
#include "autolistviewitems.h"
#include "autoprojectpart.h"
#include "autoprojectwidget.h"
#include "autodetailsview.h"

/**
* Class ProjectItem
*/

ProjectItem::ProjectItem( Type type, Q3ListView *parent, const QString &text )
		: Q3ListViewItem( parent, text ), typ( type )
{
	bld = false;
}


ProjectItem::ProjectItem( Type type, ProjectItem *parent, const QString &text )
		: Q3ListViewItem( parent, text ), typ( type )
{
	bld = false;
}


void ProjectItem::paintCell( QPainter *p, const QColorGroup &cg,
                             int column, int width, int alignment )
{
	if ( isBold() )
	{
		QFont font( p->font() );
		font.setBold( true );
		p->setFont( font );
	}
	Q3ListViewItem::paintCell( p, cg, column, width, alignment );
}


/**
* Class SubprojectItem
*/

SubprojectItem::SubprojectItem( Q3ListView *parent, const QString &text )
		: ProjectItem( Subproject, parent, text )
{
	init();
}


SubprojectItem::SubprojectItem( SubprojectItem *parent, const QString &text )
		: ProjectItem( Subproject, parent, text )
{
	init();
}


void SubprojectItem::init()
{
	targets.setAutoDelete( true );
	setPixmap( 0, SmallIcon( "folder" ) );
}


QString SubprojectItem::relativePath()
{
    QString relpath = subdir;

    SubprojectItem *it = this;
    while ( (it= dynamic_cast<SubprojectItem*>(it->parent())) )
    {
        relpath.prepend(it->subdir + "/");
    }
    relpath.remove(0, 2);

    return relpath;
}


/**
* Class TargetItem
*/

TargetItem::TargetItem( Q3ListView *lv, bool group, const QString &text )
		: ProjectItem( Target, lv, text )
{
	sources.setAutoDelete( true );
	setPixmap( 0, group ? SmallIcon( "tar" ) : SmallIcon( "binary" ) );
}


/**
* Class FileItem
*/

FileItem::FileItem( Q3ListView *lv, const QString &text, bool set_is_subst )
		: ProjectItem( File, lv, text ) , is_subst(set_is_subst)
{
	if(!is_subst)
	{
		setPixmap( 0, SmallIcon( "document" ) );
	}
	else
	{
		setPixmap( 0, SmallIcon( "variablenew" ) );
	}
}


void FileItem::changeSubstitution()
{
if(!is_subst)
return;

	bool ok;
	QString text = QInputDialog::getText(
	                   i18n("Edit Substitution"), i18n("Substitution:"), QLineEdit::Normal,
	                   name, &ok );
	if ( ok && !text.isEmpty() )
	{
		// user entered something and pressed OK
		QString new_name = text;
		if(new_name == name)
			return;
		setText(0,new_name);
		changeMakefileEntry(new_name);
		name = new_name;
	}
	else
	{
		// user entered nothing or pressed Cancel

	}
}

void FileItem::changeMakefileEntry(const QString& new_name)
{
	TargetItem* target = dynamic_cast<TargetItem*>(parent());

	QMap<QString,QString> replaceMap;

	QString canontargetname = AutoProjectTool::canonicalize(target->name);
	QString varname;
	if( target->primary == "PROGRAMS" || target->primary == "LIBRARIES" || target->primary == "LTLIBRARIES" )
		varname = canontargetname + "_SOURCES";
	else
		varname = target->prefix + "_" + target->primary;
	SubprojectItem* subProject = dynamic_cast<AutoDetailsView*>(listView())->m_part->m_widget->selectedSubproject();
	QStringList sources = QStringList::split(QRegExp("[ \t\n]"), subProject->variables[varname]);
	QStringList::iterator it = sources.find(name);
	(*it) = new_name;
	subProject->variables[varname] = sources.join(" ");
	replaceMap.insert(varname, subProject->variables[varname]);

	AutoProjectTool::modifyMakefileam(subProject->path + "/Makefile.am", replaceMap);

	if(new_name == "")
		target->sources.remove(this);

}
