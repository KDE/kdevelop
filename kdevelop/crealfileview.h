/***************************************************************************                      
    crealfileview.h - display all files and dirs in the projectdir
                             -------------------                                         

    version              :                                   
    begin                : 9 Sept 1998                                        
    copyright            : (C) 1998 by Stefan Bartel                         
    email                : bartel@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#ifndef KPROJECTDIRTREELIST_H
#define KPROJECTDIRTREELIST_H

#include <kapp.h>
#include <qwidget.h>
#include "ctreeview.h"
#include <kpopmenu.h>
#include <qstrlist.h>

//#include "cproject.h"
class CProject;

/**
  *display all files and dirs in the projectdir, a little filemanager
  @author Stefan Bartel 
  */
class CRealFileView : public CTreeView {
  Q_OBJECT 

public: 
  /** construtor */
  CRealFileView(QWidget*parent=0,const char* name=0); 

  /** destructor */
  ~CRealFileView();

  /** starts the scan */
  void refresh(CProject* project);

protected: // Implementations of virtual methods.

    /** Get the current popupmenu. */
  virtual KPopupMenu *getCurrentPopup();

signals:
  /**
   * This signal is emitted when a file is selected with the left mousebutton 
   * or keyboard. It gives back the selected filename relative to the project 
   * directory. */
  void fileSelected(QString filename);
  /**
   * This signal is emitted when the "Add to Project" entry of the context menu
   * is selected. It gives back the selected filename relative to the project
   * directory. */
  void addFileToProject(QString filename);
  /**
   * This signal is emitted when the "Remove from Project" entry of the 
   * context menu is selected. It gives back the selected filename relative to
   * the project directory. */
  void removeFileFromProject(QString filename);

  /**
   * This signal is a helper to remove the file also from the actual editlist
   */
  void removeFileFromEditlist(const QString &absfilename);

  /**
   * This signal is emitted when the "File Properties..." entry of the context
   * menu is selected. It gives back the selected filename relative to the
   * project directory. */
  void showFileProperties(QString filename);

  void commitFileToVCS(QString file);
  void updateFileFromVCS(QString file);

  void commitDirToVCS(QString dir);
  void updateDirFromVCS(QString dir);
  void menuItemHighlighted(int);
  void selectedFileNew(const char* dir);
  void selectedClassNew(const char* dir);

protected slots:
  /** emits signal "fileSelected" when it gets the selectionChanged-signal from itself */
  void slotSelectionChanged(QListViewItem* selection);
  void slotAddFileToProject();
  void slotRemoveFileFromProject();
  void slotDeleteFilePhys();
  void slotShowFileProperties();
  void slotAddToRepository();
  void slotRemoveFromRepository();
  void slotUpdate();
  void slotCommit();
  void slotShowNonPrjFiles();
  /**  */
  void slotFolderDelete();
  /**  */
  void slotFolderNew();
  /**  */
  void slotClassNew();
  /**  */
  void slotFileNew();
  void slotUpdateMakefileAm();
  void slotChangeToStatic();
  void slotChangeToShared();

private: // Popupmenus
  KPopupMenu *popup;

private: // Private attributes
  QStrList filelist;
  int file_col;
	bool showNonPrjFiles;
  QListViewItem* pRootItem;
  CProject* project;
	QString projectDir;

private: // Private methods
  /** Adds the files from a certain directory to the view. */
  void addFilesFromDir( const QString& directory, QListViewItem* parent );

  /** scans the project directory and fills the treelist */
  void scanDir(const QString& directory,QListViewItem* parent);
  QString getRelFilename(QListViewItem* pItem);
  QString getFullFilename(QListViewItem* pItem);
  bool isInstalledFile(QString filename);

};
#endif


