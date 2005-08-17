/***************************************************************************
                          framestack.h  -  description
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

#ifndef _FRAMESTACKWIDGET_H_
#define _FRAMESTACKWIDGET_H_

#include <q3listbox.h>
#include <q3strlist.h>
//Added by qt3to4:
#include <Q3CString>

namespace JAVADebugger
{

class FramestackWidget : public Q3ListBox
{
    Q_OBJECT

public:
    FramestackWidget( QWidget *parent=0, const char *name=0 );
    virtual ~FramestackWidget();

    void parseJDBBacktraceList(char *str);
    void addItem(Q3CString s);
    void updateDone();
    void clearList();
    Q3CString getFrameParams(int frame);
    QString getFrameName(int frame);

public slots:
    void slotHighlighted(int index);
    void slotSelectFrame(int frame);

signals:
    void selectFrame(int selectFrame);

private:
    int currentFrame_;
    Q3StrList *currentList_;
};

}
#endif
