/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/



#ifndef __KDEVPART_FILECREATE_H__
#define __KDEVPART_FILECREATE_H__


#include <qguardedptr.h>
#include <qptrlist.h>

#include <qwidget.h>
#include <kdevplugin.h>
#include <kdevcreatefile.h>

#include "filecreate_typechooser.h"

namespace FileCreate {
  class Widget;
  class FileType;
}

using namespace FileCreate;

class FileCreatePart : public KDevCreateFile
{
  Q_OBJECT

public:
  FileCreatePart(QObject *parent, const char *name, const QStringList &);
  virtual ~FileCreatePart();

  /**
   * Returns the list of available file types.
   */
  QPtrList<FileType> getFileTypes() const { return m_filetypes; }

  /**
   * Call this method to create a new file, within or without the project. Supply as
   * much information as you know. Leave what you don't know as QString::null.
   * The user will be prompted as necessary for the missing information, and the
   * file created, and added to the project as necessary.
   */
  virtual KDevCreateFile::CreatedFile createNewFile(QString ext = QString::null,
                     QString dir = QString::null,
                     QString name = QString::null,
                     QString subtype = QString::null);

  /**
   * Selects a docked widget.
   */
  void selectWidget(int widgetNumber);

  /**
   * Returns the current type chooser as a TypeChooser* type.
   */
  TypeChooser * typeChooserWidget() const {
    return (m_selectedWidget>=0 && m_selectedWidget<m_numWidgets) ?
                              m_availableWidgets[m_selectedWidget] : NULL;
  }

  /**
   * Returns the current type chooser as a QWidget* type.
   */
  QWidget * typeChooserWidgetAsQWidget() const {
    TypeChooser * tc = typeChooserWidget();
    return tc ? dynamic_cast<QWidget*>(tc) : NULL;
  }


  /**
   * Finds the file type object for a given extension and optionally subtype.
   * You can omit the subtype and specify the extension as ext-subtype if you wish.
   */
  FileType * getType(const QString & ext, const QString subtype = QString::null);

public slots:

  void slotProjectOpened();
  void slotProjectClosed();

  /**
   * Called when a file type is selected from the docked widget.
   */
  void slotFiletypeSelected(const FileType *);

  /**
   * Called when a new file is required - for example, from the "New.." menu action.
   */
  void slotNewFile();

protected slots:
  void slotNoteFiletype(const FileType * filetype);

private:
  /**
   * If a file has been successfully created, open it
   */
  void openCreatedFile(const KDevCreateFile::CreatedFile & createdFile);

  /**
   * Reads in file type definitions from a config DOM and adds them
   * to the file type list. If enable is true, sets them all to
   * enabled=true by default.
   */
  int readTypes(const QDomDocument & dom, bool enable);

  /**
   * The number (index to m_availableWidgets) of the widget currently in use
   */
  int m_selectedWidget;

  /**
   * List of file types from which the user can choose
   */
  QPtrList<FileType> m_filetypes;

  /**
   * List of available chooser widgets
   */
  TypeChooser * m_availableWidgets[2];

  /**
   * Number of chooser widgets
   */
  int m_numWidgets;

  /**
   * Set the current widget. Returns false if failed.
   */
  bool setWidget(TypeChooser * widget);

  /**
   * Refreshes the widget in use - i.e. updates list of filetypes in the widget.
   */
  void refresh();

  /**
   * The file type selected by the new file dialog, if appropriate.
   */
  const FileType * m_filedialogFiletype;

};


#endif
