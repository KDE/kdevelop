/***************************************************************************
                          KDevCompilerGpp/main.cpp  -  description
                             -------------------
    begin                : Thu Feb 1 2001
    copyright            : (C) 2001 by Omid Givi
    email                : omid@givi.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kdebug.h>
#include <kinstance.h>
#include "main.h"
#include "KDevCompilerGpp.h"


extern "C" {

  void *init_libKDevCompilerGpp(){
    return new GppCompilerFactory;
  }
    
};


GppCompilerFactory::GppCompilerFactory(QObject *parent, const char *name)
  : KLibFactory(parent, name){
  instance();
}


GppCompilerFactory::~GppCompilerFactory(){
  delete s_instance;
  s_instance = 0;
}


QObject *GppCompilerFactory::create(QObject *parent, const char *name,
                                   const char *classname, const QStringList &args){
  QObject *obj;
  kdDebug(9008) << "Building GppCompiler" << endl;
  obj = new KDevCompilerGpp();
  emit objectCreated(obj);
  return obj;
}


KInstance *GppCompilerFactory::s_instance = 0;
KInstance *GppCompilerFactory::instance(){
  if (!s_instance)
    s_instance = new KInstance("KDevCompilerGpp");

  return s_instance;
}
#include "main.moc"
