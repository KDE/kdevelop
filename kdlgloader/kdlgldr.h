/***************************************************************************
                          kdlgloader.h  -  description
                             -------------------
    begin                : Wed Jun  2 19:19:32 MEST 1999

    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//#define dlgldr_dbg   // This will enable debug informations

//#define dlgldr_wth   // This will enable advanced debug (watch) information
                       // You will see most method calls at StdOut if set.

#ifndef KDLGLOADER_H
#define KDLGLOADER_H

#include <kapp.h>
#include <qwidget.h>
#include <qstring.h>
#include <qtextstream.h>
#include "kdlgptrdb.h"

void WatchMsg(const char* msg);

#define BEHAVE_REPORTERRORS 1               // Report errors to user by an message box
#define BEHAVE_IGNOREERRORS 2               // Ingore errors (i.e. while reading the dialog file)

#define KDLGERR_SUCCESS                 0   // Success
#define KDLGERR_UNKNOWN                 1   // Unknown error.
#define KDLGERR_NOT_SUPPORTED           2   // Requested function is not supported.
#define KDLGERR_BAD_PARAMETER           3   // Bad parameter given.
#define KDLGERR_NAME_DOES_NOT_EXIST     4   // Specified (var)name does not exist.
#define KDLGERR_NO_NAME_SPECIFIED       5   // No (var)name specified.
#define KDLGERR_WRONG_FLAGS             6   // Wrong or unsupported flags given.
#define KDLGERR_DLG_NOT_OPENED          7   // Dialog is not open.
#define KDLGERR_ERROR_READING_FILE      8   // Error while reading the dialog file.

#define Return(errcode) { lastErrorCode = errcode; return errcode; }
#define Failed(errcode) (errcode != KDLGERR_SUCCESS)
#define LastFailed (lastErrorCode != KDLGERR_SUCCESS)
#define ReturnLastErrorCode return lastErrorCode;
#define Validate if LastFailed ReturnLastErrorCode
#define ValidateDlgOpened if (!isDialogOpened()) { Return(KDLGERR_DLG_NOT_OPENED) }
#define DoesBehave(flag) (behaviour & flag)




/**Loader class for dynamically loading dialogs from files at runtime.
  *@author Pascal Krahmer
  */
class KDlgLoader {
public:
  /** construtor */
  KDlgLoader( QWidget *Parent );
  /** destructor */
  ~KDlgLoader();

  /** Opens a dialog from a file specified by fileName. */
  long openDialog(QString fileName, long flags = 0);
  /** Frees all the dialogs memory. */
  long freeDialog(long flags = 0);

  /** Returns a pointer to an item specified by itemName. */
  QWidget* getItemPtrName(QString itemName);

  /** Returns a pointer to an item specified by itemName. */
  QWidget* getItemPtrVarName(QString itemName);

  /** Returns a pointer to an item specified by itemName. */
  QWidget* getItemPtrName(int itemId);

  /** Returns a pointer to an item specified by itemName. */
  QWidget* getItemPtrVarName(int itemId);

  /** Returns true if a dialog is opened. */
  bool isDialogOpened() { return isDlgOpened; }

  /** Changes the classes' behaviour. You can use a combination of the BEHAVE_* constants. */
  long setBehaviour(long flags = 0) { behaviour = flags; Return(KDLGERR_SUCCESS) }
  /** Returns the actually set behabiour. */
  long getBehaviour() { return behaviour; }

private:
  bool isDlgOpened;
  QString filename;
  long lastErrorCode;
  long behaviour;
  QWidget *parent;

  int dlgfilelinecnt;

  KDlgPtrDb *dbase;
  KDlgPtrDb *actdb;
protected:
  /** Displays a debug message if enabled. */
  void DebugMsg(const char* msg);
  /** Displays a watch message if enabled. */
  void WatchMsg(const char* msg);

  /** Method to read the file from hard drive. */
  long readFile(long flags = 0);
  /** reads one group from the dialog file. */
  long readGroup( QTextStream *t );
  /** reads one line from the file. */
  QString dlgReadLine( QTextStream *t );
  /** reads a complete group including subgroups and ignores it :-) */
  long readGrp_Ignore( QTextStream *t );
  /** reads the information group. */
  long readGrp_Information( QTextStream *t );
  /** reads the sessionmanagment group from the dialog file. */
  long readGrp_SessionManagement( QTextStream *t );
  /** reads an item group, creates the item and sets its' properties. */
  long readGrp_Item( QWidget* par, QTextStream *t, QString ctype, KDlgPtrDb *pardb );
  /** sets w's property name to val. */
  long setProperty(QWidget* w, QString name, QString val, QString ctype);
  /** creates a new item with the parent par from type ctype. */
  QWidget* createItem(QWidget*par, QString ctype);
  /** method to set properties of non qt widgets. */
  long setProperty_qt(QWidget* w, QString name, QString val, QString ctype);
  /** method to set properties of kde widgets */
  long setProperty_kde(QWidget* w, QString name, QString val, QString ctype);
};
#endif





























