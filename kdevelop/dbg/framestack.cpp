/***************************************************************************
                          framestack.cpp  -  description                              
                             -------------------                                         
    begin                : Sun Aug 8 1999                                           
    copyright            : (C) 1999 by John Birch
    email                : jb.nz@writeme.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "framestack.h"
#include "gdbparser.h"

#include <klocale.h>

#include <qlistbox.h>
#include <qstrlist.h>

#include <ctype.h>


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

FrameStack::FrameStack(QWidget * parent, const char * name, WFlags f) :
  QListBox (parent, name, f),
  currentFrame_(0),
  currentList_(0)
{
  connect( this,  SIGNAL(highlighted(int)), SLOT(slotHighlighted(int)));
  connect( this,  SIGNAL(selected(int)), SLOT(slotHighlighted(int)));
//  setCaption( "Frame stack" );
  show();
}

/***************************************************************************/

FrameStack::~FrameStack()
{
  delete currentList_;
}

/***************************************************************************/

void FrameStack::slotHighlighted(int frame)
{
  // always set this as the current frame and emit a signal
  // because this will display the source file if it's not visible
  // due to the user having opened a different file.
  currentFrame_ = frame;
  emit selectFrame(frame);
}

/***************************************************************************/

// someone (the vartree :-)) wants us to select this frame.
void FrameStack::slotSelectFrame(int frame)
{
  if (isSelected(frame))
    slotHighlighted(frame);   // force this when we're already selected
  else
    setCurrentItem(frame);
}

/***************************************************************************/

void FrameStack::parseGDBBacktraceList(char* str)
{
//#0  Test::Test (this=0x8073b20, parent=0x0, name=0x0) at test.cpp:224
//#1  0x804bba9 in main (argc=1, argv=0xbffff9c4) at main.cpp:24

  clear();
  delete currentList_;
  currentList_ = new QStrList(true);      // make deep copies of the data
  currentList_->setAutoDelete(true);      // delete items when they are removed

  while (char* end = strchr(str, '\n'))
  {
    *end = 0;                             // make it a string
    currentList_->append(str);            // This copies the string (deepcopies = true above)
    str = end+1;                          // next string
  }

  insertStrList(currentList_);
  currentFrame_ = 0;
}

/***************************************************************************/

QCString FrameStack::getFrameParams(int frame)
{
  if (currentList_)
  {
    if (char* frameData = currentList_->at(frame))
    {
      if (char* paramStart = strchr(frameData, '('))
      {
        GDBParser parser;
        if (char* paramEnd = parser.skipDelim(paramStart, '(', ')'))
        {
          // allow for operator()(params)
          if (paramEnd == paramStart+2)
          {
            if (*(paramEnd+1) == '(')
            {
              paramStart = paramEnd+1;
              paramEnd = parser.skipDelim(paramStart, '(', ')');
              if (!paramEnd)
                return QCString();
            }
          }

          // The parameters are contained _within_ the brackets.
          if (paramEnd-paramStart > 2)
            return QCString (paramStart+1, paramEnd-paramStart-1);
        }
      }
    }
  }

  return QCString();
}

/***************************************************************************/

QString FrameStack::getFrameName(int frame)
{
  if (currentList_)
  {
    if (char* frameData = currentList_->at(frame))
    {
      if (char* paramStart = strchr(frameData, '('))
      {
        char* fnstart = paramStart-2;
        while (fnstart > frameData)
        {
          if (isspace(*fnstart))
            break;
          fnstart--;
        }
        QString frameName(QString().sprintf("#%d %s(...)", frame,
                            QCString(fnstart, paramStart-fnstart+1).data()));
        return frameName;
      }
    }
  }

  return i18n("No stack");
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
#include "framestack.moc"
