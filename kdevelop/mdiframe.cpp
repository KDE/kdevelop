/***************************************************************************
                             mdiframe.cpp  - the MDI frame widget
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


#include "mdiframe.h"

MdiFrame::MdiFrame(QWidget *parent, const char *name)
    : QextMdiMainFrm(parent, name, 0)
{
}


QList<QextMdiChildView> MdiFrame::childrenOfType(const char *name)
{
    QList<QextMdiChildView> lst;
    
    QListIterator<QextMdiChildView> it(*m_pWinList);
    for (; it.current(); ++it)
        if (it.current()->inherits(name))
            lst.append(it.current());
    
    return lst;
}
