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

KDevFileAction::KDevFileAction (const QString& text, const QString& pix, int accel, QObject* parent, const char* name)
  : KAction(text,pix,accel,parent,name){
  m_absFileName = "not set";
}

void KDevFileAction::setAbsFileName(QString absFileName){
  m_absFileName = absFileName;
}
void KDevFileAction::setProjectName(QString projectName){
  m_projectName = projectName;
}

int KDevFileAction::plug( QWidget *widget, int index ){
  connect(this,SIGNAL(activated()),this,SLOT(slotActivated()));
  return KAction::plug( widget, index );
}

void KDevFileAction::slotActivated(){
  cerr <<  "KDevFileAction::slotActived: emit " << m_absFileName << endl;
  emit activated(m_absFileName); // fire the filename
}


