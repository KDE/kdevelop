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

#include "kextmdimainfrm.h"

class MdiFrame : public QextMdiMainFrm
{
Q_OBJECT

public:
    MdiFrame(QWidget *parent, const char *name=0);

    QList<QextMdiChildView> childrenOfType(const char *name);

public slots:
  /** redirect the slots to CKDevelop */
  virtual void switchToChildframeMode();
  virtual void switchToToplevelMode();

signals:
  void letKDevelopDoTheSwitchToChildframeMode();
  void letKDevelopDoTheSwitchToToplevelMode();
};

#endif
