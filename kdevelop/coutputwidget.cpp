/***************************************************************************
                      coutputwidget.cpp - the output window in KDevelop
                             -------------------                                         

    begin                : 5 Aug 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#include "coutputwidget.h"

#include <kapp.h>
#include <kdebug.h>

#include <qpainter.h>

COutputWidget::COutputWidget(QWidget* parent, const char* name) :
//  KEdit(parent,name)
  QMultiLineEdit(parent, name)
{
}

void COutputWidget::insertAtEnd(const QString& s)
{
  int row = (numLines() < 1)? 0 : numLines()-1;
  // correct workaround for QMultilineEdit
  //  the string inside could be NULL, and so QMultilineEdit fails
  int col = qstrlen(textLine(row));
  if (s.left(1) == "\n" && row == 0 && col == 0)
    insertAt(" "+s, row, col);
  else
    insertAt(s, row, col);
}

void COutputWidget::mouseReleaseEvent(QMouseEvent*){
  emit clicked();
}

void COutputWidget::keyPressEvent ( QKeyEvent* event){
  QMultiLineEdit::keyPressEvent(event);
  emit keyPressed(event->ascii());
}

MakeOutputItem::MakeOutputItem( const QString &text, Type type,
                                const QString& filename, int lineNo) :
  QListBoxText(text),
  m_type(type),
  m_filename(filename),
  m_lineNo(lineNo)
{}

void MakeOutputItem::paint(QPainter *p)
{
  if (!selected())
  {
    switch (m_type)
    {
      case Error:       p->setPen(Qt::darkRed);   break;
      case Diagnostic:  p->setPen(Qt::darkBlue);  break;
      default:          p->setPen(Qt::black);     break;
    }
  }
  QListBoxText::paint(p);
}

CMakeOutputWidget::CMakeOutputWidget(QWidget* parent, const char* name) :
  QListBox(parent, name),
  enterDir("[^\n]*: Entering directory `([^\n]*)'$"),
  leaveDir("[^\n]*: Leaving directory `([^\n]*)'$"),
  errorGcc("([^: \t]+):([0-9]+):.*"),
  errorJade("[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:.*")
{
  connect(this, SIGNAL(clicked(QListBoxItem*)),SLOT(slotClicked(QListBoxItem*)));
  setSelectionMode(QListBox::Extended);
}

//void CMakeOutputWidget::setCursorPosition (int, int)
//{
//}

//QString CMakeOutputWidget::textLine (int)
//{
//  return QString::null;
//}

//void CMakeOutputWidget::cursorPosition (int *, int *)
//{
//}

void CMakeOutputWidget::insertAtEnd(const QString& text)
{
  buf += text;
  int pos;

  while ( (pos = buf.find('\n')) != -1)
  {
    processLine(buf.left(pos));
    buf.remove(0, pos+1);
  }
}

void CMakeOutputWidget::processLine(const QString& line)
{
  QString fn = QString::null;
  int row = -1;
  MakeOutputItem::Type type = MakeOutputItem::Normal;

  const int errorGccFileGroup = 1;
  const int errorJadeFileGroup = 1;
  const int errorGccRowGroup = 2;
  const int errorJadeRowGroup = 2;

  if (enterDir.match(line))
  {
    QString *dir = new QString(enterDir.group(1));
    dirStack.push(dir);
  }
  else
  {
    if (leaveDir.match(line))
    {
      QString *dir = dirStack.pop();
      delete dir;
    }
    else
    {
      if (errorGcc.match(line))
      {
        type = MakeOutputItem::Error;
        fn = errorGcc.group(errorGccFileGroup);
        row = QString(errorGcc.group(errorGccRowGroup)).toInt()-1;
        if (dirStack.top())
          fn.prepend("/").prepend(*dirStack.top());
      }
      else
      {
        if (errorJade.match(line))
        {
          type = MakeOutputItem::Error;
          fn = errorGcc.group(errorJadeFileGroup);
          row = QString(errorJade.group(errorJadeRowGroup)).toInt()-1;
          if (dirStack.top())
            fn.prepend("/").prepend(*dirStack.top());
        }
      }
    }
  }


  MakeOutputItem* item = new MakeOutputItem(line, type, fn, row);
  insertItem(item);
  setBottomItem(numRows()-1);
}

//void CMakeOutputWidget::mouseReleaseEvent(QMouseEvent*)
//{
//  emit clicked();
//}
//
//void CMakeOutputWidget::keyPressEvent ( QKeyEvent* event)
//{
//  emit keyPressed(event->ascii());
//}
//
void CMakeOutputWidget::start()
{
  clear();
  dirStack.clear();
}

void CMakeOutputWidget::viewNextError()
{
  for (int i = currentItem()+1; i < numRows(); ++i)
  {
    MakeOutputItem* makeItem = static_cast<MakeOutputItem*>(item(i));
    if (makeItem && makeItem->lineNo() != -1)
    {
      clearSelection();
      setCurrentItem(makeItem);
      setSelected(i, true);
      emit switchToFile(makeItem->filename(), makeItem->lineNo());
      return;
    }
  }

  kapp->beep();
}

void CMakeOutputWidget::viewPreviousError()
{
  int cur = (currentItem() == -1)? numRows() : currentItem()-1;
  for (int i = cur; i >= 0; --i)
  {
    MakeOutputItem* makeItem = static_cast<MakeOutputItem*>(item(i));
    if (makeItem && makeItem->lineNo() != -1)
    {
      clearSelection();
      setCurrentItem(makeItem);
      setSelected(i,true);
      emit switchToFile(makeItem->filename(), makeItem->lineNo());
      return;
    }
  }
  kapp->beep();
}

void CMakeOutputWidget::slotClicked (QListBoxItem* item)
{
  MakeOutputItem* makeItem = static_cast<MakeOutputItem*>(item);
  if (makeItem && makeItem->lineNo() != -1)
    emit switchToFile(makeItem->filename(), makeItem->lineNo());
}

#include "coutputwidget.moc"
