/***************************************************************************
                             outputview.h
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

#ifndef _APPOUTPUTWIDGET_H_
#define _APPOUTPUTWIDGET_H_

#include "processview.h"


class AppOutputWidget : public ProcessView
{
    Q_OBJECT
    
public:
    AppOutputWidget();
    ~AppOutputWidget();

    void compilationAborted();
    
private:
    virtual void childFinished(bool normal, int status);
};

#endif
