/***************************************************************************
                          framestack.cpp  -  description
                             -------------------
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "framestackwidget.h"
#include "jdbparser.h"

#include <klocale.h>

#include <q3listbox.h>
#include <q3strlist.h>
//Added by qt3to4:
#include <Q3CString>

#include <ctype.h>


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace JAVADebugger
{

FramestackWidget::FramestackWidget(QWidget *parent, const char *name)
    : Q3ListBox(parent, name),
      currentFrame_(0),
      currentList_(0)
{
    connect( this, SIGNAL(highlighted(int)), SLOT(slotHighlighted(int)) );
    connect( this, SIGNAL(selected(int)), SLOT(slotHighlighted(int)) );
}


/***************************************************************************/

FramestackWidget::~FramestackWidget()
{
    delete currentList_;
}

/***************************************************************************/

void FramestackWidget::slotHighlighted(int frame)
{
    // always set this as the current frame and emit a signal
    // because this will display the source file if it's not visible
    // due to the user having opened a different file.
    currentFrame_ = frame;
    emit selectFrame(frame);
}

/***************************************************************************/

// someone (the vartree :-)) wants us to select this frame.
void FramestackWidget::slotSelectFrame(int frame)
{
  if (isSelected(frame))
    slotHighlighted(frame);   // force this when we're already selected
  else
    setCurrentItem(frame);
}

/***************************************************************************/

void FramestackWidget::clearList() {
    clear();
    delete currentList_;
    currentList_ = new Q3StrList(true);      // make deep copies of the data
    currentList_->setAutoDelete(true);      // delete items when they are removed
}

void FramestackWidget::addItem(Q3CString s) {
        currentList_->append(s);            // This copies the string (deepcopies = true above)
}

void FramestackWidget::parseJDBBacktraceList(char */*str*/)
{
}

void FramestackWidget::updateDone()
{

    insertStrList(currentList_);
    currentFrame_ = 0;
}

/***************************************************************************/

Q3CString FramestackWidget::getFrameParams(int frame)
{
    if (!currentList_) {
        if (char *frameData = currentList_->at(frame)) {
            if (char *paramStart = strchr(frameData, '(')) {
                JDBParser parser;
                if (char *paramEnd = parser.skipDelim(paramStart, '(', ')')) {
                    // allow for operator()(params)
                    if (paramEnd == paramStart+2) {
                        if (*(paramEnd+1) == '(') {
                            paramStart = paramEnd+1;
                            paramEnd = parser.skipDelim(paramStart, '(', ')');
                            if (!paramEnd)
                                return Q3CString();
                        }
                    }

                    // The parameters are contained _within_ the brackets.
                    if (paramEnd-paramStart > 2)
                        return Q3CString (paramStart+1, paramEnd-paramStart-1);
                }
            }
        }
    }

    return Q3CString();
}

/***************************************************************************/

QString FramestackWidget::getFrameName(int frame)
{
    if (currentList_) {
        if (char *frameData = currentList_->at(frame)) {
            if (char *paramStart = strchr(frameData, '(')) {
                char *fnstart = paramStart-2;
                while (fnstart > frameData) {
                    if (isspace(*fnstart))
                        break;
                    fnstart--;
                }
                QString frameName(QString().sprintf("#%d %s(...)", frame,
                                                    Q3CString(fnstart, paramStart-fnstart+1).data()));
                return frameName;
            }
        }
    }

    return i18n("No stack");
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#include "framestackwidget.moc"
