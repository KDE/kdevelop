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

#ifndef _AUTOPROJECTWIDGET_H_
#define _AUTOPROJECTWIDGET_H_

#include <qlayout.h>
#include <qdict.h>
#include <qlistview.h>
#include <qmap.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qtoolbutton.h>
#include <kiconloader.h>
#include <klocale.h>

#include "domutil.h"

class AutoProjectPart;
class AutoSubprojectView;
class AutoDetailsView;
class SubprojectItem;
class TargetItem;
class FileItem;
class KAction;
class QDomElement;
class QToolButton;
class QStringList;


class AutoProjectWidget : public QVBox
{
	Q_OBJECT
	friend class RemoveFileDialog;
	friend class TargetOptionsDialog;  // to access projectDom() via m_part->projectDom()
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
	QString projectDirectory() const;
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
        * If it does contain "/" character(s), ... @todo .. add to appropriate target
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
				     const QString &prefix, const QString &primary, 
				     bool take=true );
	/**
	* Creates a FileItem instance without a parent item.
	*/
	FileItem *createFileItem(const QString &name, SubprojectItem *subproject);

	/**
	 * Returns the Subproject that contains the Active Target. The Active Target is a special target
	 * to which e.g. all files are added to.
	 */
	SubprojectItem* activeSubproject ();
	void setActiveSubproject( SubprojectItem* spitem );

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
         * Returns the projectdir-relative path for a target item
         */
        QString pathForTarget(const TargetItem *item) const;

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
	
	AutoSubprojectView* getSubprojectView ();
	AutoDetailsView* getDetailsView ();
		

	void emitAddedFile ( const QString& name );
	void emitAddedFiles(const QStringList &fileList);
	void emitRemovedFile ( const QString& name );
	void emitRemovedFiles(const QStringList &fileList);

	void parse(SubprojectItem *item);
 
        enum AutoProjectView { SubprojectView, DetailsView };
        void setLastFocusedView(AutoProjectView view);

public slots:
	void slotOverviewSelectionChanged(QListViewItem *item);
	void slotDetailsSelectionChanged(QListViewItem *item);

protected:
	void initOverview ( QWidget* parent );
	void initDetailview ( QWidget* parent );
	void initActions ();
        
        virtual void focusInEvent(QFocusEvent *e);

private:

	QToolButton *subProjectOptionsButton, *addSubprojectButton, *addExistingSubprojectButton;
	QToolButton *addTargetButton, *addServiceButton, *addApplicationButton;
	QToolButton *buildSubprojectButton, *targetOptionsButton;
	QToolButton *addNewFileButton, *addExistingFileButton;
	QToolButton *buildTargetButton, *executeTargetButton, *removeButton;

	AutoSubprojectView* m_subprojectView;
	AutoDetailsView* m_detailView;
	
	bool m_kdeMode;
	AutoProjectPart *m_part;
	SubprojectItem *m_activeSubproject;
	TargetItem *m_activeTarget;
	TargetItem *m_choosenTarget;
	SubprojectItem *m_shownSubproject;
 
        AutoProjectView m_lastFocusedView;
};

#endif
