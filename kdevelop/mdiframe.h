/***************************************************************************
                             mdiframe.h  - the MDI frame widget
                             -------------------                                         
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef _MDIFRAME_H_
#define _MDIFRAME_H_

#include "widgets/qextmdi/qextmdimainfrm.h"

class MdiFrame : public QextMdiMainFrm
{
public:
    MdiFrame(QWidget *parent, const char *name=0);

    QList<QextMdiChildView> childrenOfType(const char *name);
};

#endif
