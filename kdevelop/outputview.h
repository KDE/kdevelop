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

#ifndef OUTPUTVIEW_H
#define OUTPUTVIEW_H

#include "processview.h"


class OutputView : public ProcessView
{
    Q_OBJECT
public:
    OutputView(QWidget *parent, const char *name=0);
    ~OutputView();

protected:
    virtual void childFinished(bool normal, int status);

    // Component notifications:
    virtual void compilationAborted();
};

#endif
