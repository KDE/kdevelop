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

#ifndef _AUTOPROJECTWIDGET_H_
#define _AUTOPROJECTWIDGET_H_

#include <qlayout.h>
#include <qdict.h>
#include <qlistview.h>
#include <qmap.h>
//#include <qstrlist.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qtoolbutton.h>
#include <kiconloader.h>
#include <klocale.h>

class AutoProjectPart;
class KListView;
class SubprojectItem;
class TargetItem;
class FileItem;
class KPopupMenu;
class KAction;
class QDomElement;
class QToolButton;
class QStringList;

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

	/* name of the directory */
	QString subdir;
	/* absolute path */
	QString path;
	/* mapping from prefix to path */
	QMap<QString, QString> prefixes;
	/* mapping from variable name to value */
	QMap<QString, QString> variables;
	/* list of targets */
	QPtrList<TargetItem> targets;

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

	// Target kind - not used currently
	//    TargetKind kind;
	// Name of target, e.g. foo
	QString name;
	// One of PROGRAMS, LIBRARIES, LTLIBRARIES, SCRIPTS, HEADERS, DATA, JAVA
	// In addition to these automake primaries, we use KDEICON and KDEDOCS
	// for am_edit magic
	QString primary;
	// May be bin, pkglib, noinst, check, sbin, pkgdata, java...
	QString prefix;
	// Content of foo_SOURCES (or java_JAVA) assignment
	QPtrList<FileItem> sources;
	// Content of foo_LDFLAGS assignment
	QString ldflags;
	// Content of foo_LDADD assignment
	QString ldadd;
	// Content of foo_LIBADD assignment
	QString libadd;
	// Content of foo_DEPENDENCIES assignment
	QString dependencies;
};


// Not sure if this complexity is really necessary...
class FileItem : public ProjectItem
{
public:
	FileItem(QListView *lv, const QString &text);

	QString name;
};


class AutoProjectWidget : public QVBox
{
	Q_OBJECT

public: 
	AutoProjectWidget(AutoProjectPart *part, bool kde);
	~AutoProjectWidget();

	void openProject(const QString &dirName);
	void closeProject();

	/**
	* A list of the (relative) names of all subprojects (== subdirectories)
	*/
	QStringList allSubprojects();
	/**
	 * A list of all Subproject items in the overview KListView
	 */
	QPtrList <SubprojectItem> allSubprojectItems();
	/**
	* A list of the (relative) names of all libraries
	*/
	QStringList allLibraries();
	/**
	* A list of all files that belong to the project
	**/
	QStringList allFiles();
	/**
	* The top level directory of the project.
	**/
	QString projectDirectory();
	/**
	* The directory of the currently shown subproject.
	*/
	QString subprojectDirectory();
	/**
	* Are we in KDE mode?
	*/
	bool kdeMode() const
	{ return m_kdeMode; }

	/**
	* Sets the given target active. The argument is given
	* relative to the project directory.
	*/
	void setActiveTarget(const QString &targetPath);
	/**
	* Returns the active target as path relative to
	* the project directory.
	*/
	QString activeDirectory();

	/**
	* Adds a file to the active target.
        * If the file does not contain a "/" character, it is added
        * to the active target.
        * If it does contain "/" character(s), ... TODO.. add to appropriate target
	*/
	void addFiles(const QStringList &list);
	/**
	* Removes the file fileName from the directory directory.
	* (not implemented currently)
	*/
	void removeFiles(const QStringList &list);

	/**
	* Returns the currently selected target. Returns 0 if
	* no target is selected.
	*/
	TargetItem *selectedTarget();

	/**
	* Returns the currently selected file. Returns 0 if
	* no file is selected.
	*/
	FileItem *selectedFile();

	/**
	* Returns the currently selected subproject (directory with Makefile.am). Returns 0 if
	* no subproject is selected.
	*/
	SubprojectItem* selectedSubproject();

	/**
	* Creates a TargetItem instance without a parent item.
	*/
	TargetItem *createTargetItem(const QString &name,
								const QString &prefix, const QString &primary);
	/**
	* Creates a FileItem instance without a parent item.
	*/
	FileItem *createFileItem(const QString &name);
	
	/**
	 * Returns the Subproject that contains the Active Target. The Active Target is a special target
	 * to which e.g. all files are added to.
	 */
	SubprojectItem* activeSubproject ();
	
	/**
	 * Returns the Active Target. The Active Target is a special target
	 * to which e.g. all files are added to.
	 */
	TargetItem* activeTarget();
        
        /**
         * Returns the sub project item, if any, for a given path. The path supplied can be either
         * absolute, or relative to the project directory. If no subproject item is found for the
         * path, null is returned.
         */
        SubprojectItem* subprojectItemForPath(const QString & path, bool pathIsAbsolute=false);

        /**
         * Adds file fileName to target titem in subproject spitem
         */
        void addToTarget(const QString & fileName, SubprojectItem* spitem, TargetItem* titem);
  
        /**
	 * Restores the last settings of the AutoProjectWidget
	 */
	void restoreSession ( const QDomElement* el );
	
	/**
	 * Saves the latest changes of the AutoProjectWidget to the session file.
	 */
	void saveSession ( QDomElement* el );

	void emitAddedFile ( const QString& name );
	void emitAddedFiles(const QStringList &fileList);
	void emitRemovedFile ( const QString& name );
	void emitRemovedFiles(const QStringList &fileList);

	void parse(SubprojectItem *item);

private slots:
	void slotOverviewSelectionChanged(QListViewItem *item);
	void slotOverviewContextMenu(KListView *, QListViewItem *item, const QPoint &p);
	void slotDetailsSelectionChanged(QListViewItem *item);
	void slotDetailsExecuted(QListViewItem *item);
	void slotDetailsContextMenu(KListView *, QListViewItem *item, const QPoint &p);

	void slotAddApplication();
	void slotSubprojectOptions();
	void slotAddSubproject();
	void slotAddExistingSubproject();
	void slotAddTarget();
	void slotAddService();
	void slotBuildSubproject();
	void slotTargetOptions ();
	void slotAddNewFile();
	void slotAddExistingFile();
	void slotAddIcon();
	void slotBuildTarget();
	void slotRemoveDetail();
	void slotRemoveSubproject();
	void slotSetActiveTarget();

protected:
	void parseKDEDOCS(SubprojectItem *item,
					const QString &lhs, const QString &rhs);
	void parseKDEICON(SubprojectItem *item,
					const QString &lhs, const QString &rhs);
	void parsePrimary(SubprojectItem *item,
					const QString &lhs, const QString &rhs);
	void parsePrefix(SubprojectItem *item,
					const QString &lhs, const QString &rhs);
	void parseSUBDIRS(SubprojectItem *item,
					const QString &lhs, const QString &rhs);

	void initOverview ( QWidget* parent );
	void initDetailview ( QWidget* parent );
	void initActions ();

private:

	QToolButton *subProjectOptionsButton, *addSubprojectButton, *addExistingSubprojectButton;
	QToolButton *addTargetButton, *addServiceButton, *addApplicationButton;
	QToolButton *buildSubprojectButton, *targetOptionsButton;
	QToolButton *addNewFileButton, *addExistingFileButton;
	QToolButton *buildTargetButton, *removeButton;

	KAction* addApplicationAction;
	KAction* subProjectOptionsAction;
	KAction* addSubprojectAction;
	KAction* addExistingSubprojectAction;
	KAction* addTargetAction;
	KAction* addServiceAction;
	KAction* buildSubprojectAction;
	KAction* targetOptionsAction;
	KAction* addNewFileAction;
	KAction* addExistingFileAction;
	KAction* addIconAction;
	KAction* buildTargetAction;
	KAction* setActiveTargetAction;

	KAction* removeDetailAction;
	KAction* removeSubprojectAction;

	KListView *overview;
	KListView *details;
	
	bool m_kdeMode;
	AutoProjectPart *m_part;
	SubprojectItem *m_activeSubproject;
	TargetItem *m_activeTarget;
	TargetItem *m_choosenTarget;
	SubprojectItem *m_shownSubproject;
};

#endif
