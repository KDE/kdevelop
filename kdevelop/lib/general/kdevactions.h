/***************************************************************************
 *   Copyright (C) 2000 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVACTIONS_H_
#define _KDEVACTIONS_H_

#include <qlist.h>
#include <qstring.h>
#include <kaction.h>
#include "kdevnodes.h"


class KDevNodeAction : public KAction {
  Q_OBJECT
    public:
  KDevNodeAction( KDevNode* pNode,const QString& text, const QString& pix, int accel = 0, QObject* parent = 0, const char* name = 0 );
  KDevNodeAction( KDevNode* pNode,const QString& text,int accel = 0, QObject* parent = 0, const char* name = 0 );
  KDevNodeAction( KDevNode* pNode,const QString& text,
                  QObject* receiver, const char* slot,
                  QObject* parent, const char* name );
  KDevNode kdevNode();
  virtual ~KDevNodeAction() {}

 signals:
  /** connect to this*/
  void activated(KDevNode* pNode);
  
  protected slots:
    virtual void slotActivated();
  
 private:
  KDevNode* m_pNode;
};

#endif
