/***************************************************************************
                             outputview.cpp
                             -------------------                                         

    copyright            : (C) 1999 The KDevelop Team
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include <klocale.h>
#include "outputview.h"


OutputView::OutputView(QWidget *parent, const char *name)
    : ProcessView(parent, name)
{
}


OutputView::~OutputView()
{}


void OutputView::compilationAborted()
{
    if (isRunning())
        {
            killJob();
            insertItem(i18n("Killed process."));
        }
}
