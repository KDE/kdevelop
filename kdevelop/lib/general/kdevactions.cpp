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


#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <iostream.h>
#include <qfont.h>
#include <kaction.h>
#include "kdevactions.h"

KDevNodeAction::KDevNodeAction (KDevNode* pNode,const QString& text, const QString& pix, int accel, QObject* parent, const char* name)
  : KAction(text,pix,accel,parent,name){
  m_pNode = pNode;
  connect(this,SIGNAL(activated()),this,SLOT(slotActivated()));
}

KDevNodeAction::KDevNodeAction(KDevNode* pNode, const QString& text,int accel, QObject* parent, const char* name)
  : KAction(text,accel,parent,name) {
  m_pNode = pNode;
  connect(this,SIGNAL(activated()),this,SLOT(slotActivated()));
}

KDevNodeAction::KDevNodeAction(KDevNode* pNode, const QString& text,
                               QObject* receiver, const char* slot,
                               QObject* parent, const char* name)
  : KAction(text,0,parent,name) {
  m_pNode = pNode;
  connect(this,SIGNAL(activated()),this,SLOT(slotActivated()));
  if (receiver)
      connect( this, SIGNAL(activated(KDevNode*)), receiver, slot );
}

void KDevNodeAction::slotActivated(){
  cerr <<  "KDevNodeAction::slotActived: emited " << endl;
  //  m_pNode->show();
  emit activated(m_pNode); // fire the node
}


#include "kdevactions.moc"
