/***************************************************************************
                             makeview.h
                             -------------------                                         
    copyright            : (C) 1999 by The KDevelop Team
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef MAKEVIEW_H
#define MAKEVIEW_H

#include "processview.h"


class MakeView : public ProcessView
{
    Q_OBJECT
public:
    MakeView(QWidget *parent, const char *name=0);
    ~MakeView();

    // Component notifications:
    //    virtual void compilationAborted();
    //    virtual void projectOpened(CProject *prj);
    virtual void projectClosed();
};

#endif
