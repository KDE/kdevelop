/***************************************************************************
*   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   Copyright (C) 2002 by Victor Röder                                    *
*   victor_roeder@gmx.de                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef AUTOLISTVIEWITEMS_H
#define AUTOLISTVIEWITEMS_H

#include <qptrlist.h>

#include <qlistview.h>

class TargetItem;
class FileItem;
class AutoProjectPart;

/**
* Base class for all items appearing in ProjectOverview and ProjectDetails.
*/
class ProjectItem : public QListViewItem
{
public:
	enum Type { Subproject, Target, File };

	ProjectItem(Type type, QListView *parent, const QString &text);
	ProjectItem(Type type, ProjectItem *parent, const QString &text);

	void paintCell(QPainter *p, const QColorGroup &cg,
				int column, int width, int alignment);
	void setBold(bool b)
	{ bld = b; }
	bool isBold() const
	{ return bld; }
	Type type()
	{ return typ; }

private:
	Type typ;
	bool bld;
	
};


/**
* Stores the content of one Makefile.am
*/
class SubprojectItem : public ProjectItem
{
public:
	SubprojectItem(QListView *parent, const QString &text);
	SubprojectItem(SubprojectItem *parent, const QString &text);
	
	/** name of the directory */
	QString subdir;
	/** absolute path */
	QString path;
	/** mapping from prefix to path */
	QMap<QString, QString> prefixes;
	/** mapping from variable name to value */
	QMap<QString, QString> variables;
	/** list of targets */
	QPtrList<TargetItem> targets;

	QString relativePath();

private:
	void init();
};


/**
* Stores one target
* For e.g. the line
*    bin_LTLIBRARIES = foo.la
* generates a target with name 'foo.la', primary LTLIBRARIES and prefix 'bin'
* In order to make things not too simple ;-) headers and data are handled
* a bit different from programs, libraries and scripts: All headers for a
* certain prefix (analogously for data) are put in _one_ TargetItem object,
* and the names of the files are put in the sources variable. This avoids
* cluttering the list view with lots of header items.
*/
class TargetItem : public ProjectItem
{
public:
	//    enum TargetKind { Program, Library, DataGroup, IconGroup, DocGroup };

	TargetItem(QListView *lv, bool group, const QString &text);

	//! Target kind - not used currently
	//!    TargetKind kind;
	//! Name of target, e.g. foo
	QString name;
	//! One of PROGRAMS, LIBRARIES, LTLIBRARIES, SCRIPTS, HEADERS, DATA, JAVA
	//! In addition to these automake primaries, we use KDEICON and KDEDOCS
	//! for am_edit magic
	QString primary;
	//! May be bin, pkglib, noinst, check, sbin, pkgdata, java...
	QString prefix;
	//! Content of foo_SOURCES (or java_JAVA) assignment
	QPtrList<FileItem> sources;
	//! Content of foo_LDFLAGS assignment
	QString ldflags;
	//! Content of foo_LDADD assignment
	QString ldadd;
	//! Content of foo_LIBADD assignment
	QString libadd;
	//! Content of foo_DEPENDENCIES assignment
	QString dependencies;
};


// Not sure if this complexity is really necessary...
class FileItem : public ProjectItem
{

public:
	FileItem(QListView *lv, const QString &text, bool set_is_subst = false );
	void changeSubstitution();
	void changeMakefileEntry(const QString&);

	QString name;
	QString uiFileLink;
	const bool is_subst;
};

#endif

