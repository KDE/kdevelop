/*
  $Id$

  Copyright (C) 1999 Glen Parker <glenebob@nwlink.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.

  --------------------------------------------------------------------

  This implements a dialog used to display and control undo/redo history.
  It uses a specialized QListBox subclass to provide a selection mechanism
  that will:
  1) always have the first item selected, and
  2) maintain a contiguous multiple selection
*/

#ifndef __undohistory_h_
#define __undohistory_h_

#include <qdialog.h>
#include <qlistbox.h>

#include "kwview.h"

class UndoListBox;

// the dialog class that provides the interface to the user
class UndoHistory : public QDialog
{
  Q_OBJECT

  public:
    /**
      Constructed just like a regular QDialog
    */
    UndoHistory(KWrite*, QWidget *parent=0, const char *name=0, bool modal=FALSE, WFlags f=0);
    virtual ~UndoHistory();

  public slots:
    /**
      This should be called whenever a change occurs in the undo/redo list.
      Causes the dialog to update its interface.
    */
    void newUndo();

  signals:
    /**
      Emitted when the user hits the Undo button.  Specifies the number of
      operations to undo.
    */
    void undo(int);
    /**
      Emitted when the user hits the Redo button.  Specifies the number of
      undone operations to redo.
    */
    void redo(int);

  protected:
    KWrite        *kWrite;

    UndoListBox   *lbUndo,
                  *lbRedo;
    QPushButton   *btnUndo,
                  *btnRedo;

  protected slots:
    void slotUndo();
    void slotRedo();
    void slotUndoSelChanged(int);
    void slotRedoSelChanged(int);

};

// listbox class used to provide contiguous, 0-based selection
// this is used internally
class UndoListBox : public QListBox
{
  Q_OBJECT

  public:
    UndoListBox(QWidget * parent=0, const char * name=0, WFlags f=0);
    virtual ~UndoListBox();

    int selCount();
    void setSelCount(int count);

    void insertItem (const QString &text, int index = -1);
    void removeItem (int index);
    void clear();

  protected:
    int   _selCount;

  signals:
    void sigSelected(int);

  protected slots:
    void _slotSelectionChanged();

};

#endif
