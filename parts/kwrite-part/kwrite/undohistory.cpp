/*
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
  that must:
  1) always have the first item selected, and
  2) maintain a contiguous multiple selection
*/

#include <stdio.h>

#include <qwidget.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>

#include <klocale.h>

#include "kwview.h"

#include "undohistory.h"
#include "undohistory.moc"

/////////////////////////////////////////////////////////////////////
// UndoHistory implementation
//
UndoHistory::UndoHistory(KWrite *kWrite, QWidget *parent, const char *name, bool modal, WFlags f)
  : QDialog(parent, name, modal, f)
{
  this->kWrite = kWrite;

  QPushButton     *btn;
  QLabel          *lbl;
  QHBoxLayout     *hLayout;
  QVBoxLayout     *vLayout;

  hLayout = new QHBoxLayout(this, 5, 4);

  vLayout = new QVBoxLayout(hLayout);
  lbl = new QLabel(i18n("Undo List"), this);
  lbUndo = new UndoListBox(this);
  vLayout->addWidget(lbl);
  vLayout->addWidget(lbUndo);

  vLayout = new QVBoxLayout(hLayout);
  lbl = new QLabel(i18n("Redo List"), this);
  lbRedo = new UndoListBox(this);
  vLayout->addWidget(lbl);
  vLayout->addWidget(lbRedo);

  lbUndo->setMinimumSize(QSize(150,140));
  lbRedo->setMinimumSize(QSize(150,140));

  connect(lbUndo, SIGNAL(sigSelected(int)), this, SLOT(slotUndoSelChanged(int)));
  connect(lbRedo, SIGNAL(sigSelected(int)), this, SLOT(slotRedoSelChanged(int)));

  vLayout = new QVBoxLayout(hLayout);

  btnUndo = new QPushButton(this);
  btnUndo->setText(i18n("&Undo"));
  btnUndo->setEnabled(false);
  btnUndo->setFixedSize(btnUndo->sizeHint());
  connect(btnUndo, SIGNAL(clicked()), this, SLOT(slotUndo()));

  vLayout->addWidget(btnUndo, 0);

  btnRedo = new QPushButton(this);
  btnRedo->setText(i18n("&Redo"));
  btnRedo->setEnabled(false);
  btnRedo->setFixedSize(btnRedo->sizeHint());
  connect(btnRedo, SIGNAL(clicked()), this, SLOT(slotRedo()));

  vLayout->addWidget(btnRedo, 0);

  btn = new QPushButton(this);
  btn->setText(i18n("&Close"));
  btn->setFixedSize(btn->sizeHint());
  connect(btn, SIGNAL(clicked()), this, SLOT(close()));

  vLayout->addWidget(btn, 0, AlignBottom);

  newUndo();
}

UndoHistory::~UndoHistory()
{}

void UndoHistory::newUndo()
{
  QValueList<int> undoList;
  QValueList<int>::Iterator it;

  // we don't want a signal storm...
  disconnect(lbUndo, SIGNAL(sigSelected(int)), this, SLOT(slotUndoSelChanged(int)));
  disconnect(lbRedo, SIGNAL(sigSelected(int)), this, SLOT(slotRedoSelChanged(int)));

  kWrite->undoTypeList(undoList);

  lbUndo->clear();

  for (it = undoList.begin() ; it != undoList.end() ; it++) {
    lbUndo->insertItem(i18n(kWrite->undoTypeName(*it)));
  }

  kWrite->redoTypeList(undoList);

  lbRedo->clear();
  for (it = undoList.begin() ; it != undoList.end() ; it++) {
    lbRedo->insertItem(i18n(kWrite->undoTypeName(*it)));
  }

  connect(lbUndo, SIGNAL(sigSelected(int)), this, SLOT(slotUndoSelChanged(int)));
  connect(lbRedo, SIGNAL(sigSelected(int)), this, SLOT(slotRedoSelChanged(int)));

  slotUndoSelChanged(lbUndo->selCount());
  slotRedoSelChanged(lbRedo->selCount());
}

void UndoHistory::slotUndo()
{
  int selCount = lbUndo->selCount();
  emit undo(selCount);
  lbRedo->setSelCount(selCount);
}
void UndoHistory::slotRedo()
{
  int selCount = lbRedo->selCount();
  emit redo(selCount);
  lbUndo->setSelCount(selCount);
}

void UndoHistory::slotUndoSelChanged(int cnt)
{
  btnUndo->setEnabled(cnt > 0);
}

void UndoHistory::slotRedoSelChanged(int cnt)
{
  btnRedo->setEnabled(cnt > 0);
}

/////////////////////////////////////////////////////////////////////
// UndoListBox implementation
//
UndoListBox::UndoListBox(QWidget *parent, const char *name, WFlags f)
  : QListBox(parent, name, f)
{
  _selCount = 0;
  setSelectionMode(Extended);
  connect(this, SIGNAL(highlighted(int)), this, SLOT(_slotSelectionChanged()));
  connect(this, SIGNAL(selectionChanged()), this, SLOT(_slotSelectionChanged()));
}

UndoListBox::~UndoListBox()
{}

void UndoListBox::insertItem (const QString &text, int index)
{
  bool sig = false;

  if (count() == 0)
    sig = true;
  else if (index > -1)
    sig = (isSelected(index));

  QListBox::insertItem(text, index);

  if (sig)
    _slotSelectionChanged();
}

void UndoListBox::removeItem (int index)
{
  bool sig;

  if (count() == 1)
    sig = true;
  else if (index == -1)
    sig = (isSelected(count() - 1));
  else
    sig = (isSelected(index));

  QListBox::removeItem(index);

  if (sig)
    _slotSelectionChanged();
}

void UndoListBox::clear()
{
  bool sig = (count() > 0);

  QListBox::clear();

  if (sig)
    _slotSelectionChanged();
}

int UndoListBox::selCount()
{
  return _selCount;
}

void UndoListBox::setSelCount(int count)
{
  if (count == _selCount)
    return;

  if (count < 1 || count > (int)this->count())
    return;

  setCurrentItem(count - 1);
}

// make sure the first item is selected, and that there are no holes
void UndoListBox::_slotSelectionChanged()
{
  int     count = this->count();

  if (! count) {
    if (_selCount != 0) {
      _selCount = 0;
      emit sigSelected(_selCount);
    }
    return;
  }

  if (currentItem() < 0)
    setCurrentItem(0);

  int     i;
  int     currItem = currentItem();
  int     max = (currItem+1 > _selCount ? currItem+1 : _selCount);

  for (i = 0 ; i < max ; i++) {
    if (i > currItem) {
      if (isSelected(i)) {
        setSelected(i, false);
      }
    } else {
      if (! isSelected(i)) {
        setSelected(i, true);
      }
    }
  }

  if (_selCount != currItem + 1) {
    _selCount = currItem + 1;
    emit sigSelected(_selCount);
  }
}
