/***************************************************************************
                          kdebinaryprojectplugin.cpp  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdebinaryprojectplugin.h"
#include <iostream.h>
#include <kiconloader.h>
#include <klocale.h>

KDEBinaryProjectPlugin::KDEBinaryProjectPlugin(QObject *parent, const char *name) 
  : Project(parent,name){
  cerr  << "enter KDEBinaryProjectPlugin::KDEBinaryProjectPlugin" << endl;
  m_pAboutData=0;
}
KDEBinaryProjectPlugin::~KDEBinaryProjectPlugin(){
}
KAboutData* KDEBinaryProjectPlugin::aboutPlugin(){
  if (m_pAboutData == 0){
    m_pAboutData= new KAboutData( "KDEBinaryProject", I18N_NOOP("KDE Project (Binary)"),
				  "0.1", "desc",
				  KAboutData::License_GPL,
				  "(c) 1998-2000, The KDevelop Team",
				  "text",
				  "http://www.kdevelop.org");

    m_pAboutData->addAuthor("Sandy Meier",I18N_NOOP("Developer"), "smeier@kdevelop.org");
  }
  return m_pAboutData;
}
