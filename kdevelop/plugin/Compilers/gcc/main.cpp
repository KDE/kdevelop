/***************************************************************************
                          KDevCompilerGCC/main.cpp  -  description
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
#include "KDevCompilerGcc.h"


extern "C" {

  void *init_libKDevCompilerGcc(){
    return new GccCompilerFactory;
  }
    
};


GccCompilerFactory::GccCompilerFactory(QObject *parent, const char *name)
  : KLibFactory(parent, name){
  instance();
}


GccCompilerFactory::~GccCompilerFactory(){
  delete s_instance;
  s_instance = 0;
}


QObject *GccCompilerFactory::create(QObject *parent, const char *name,
                                   const char *classname, const QStringList &args){
  QObject *obj;
  kdDebug(9008) << "Building GccCompiler" << endl;
  obj = new KDevCompilerGcc();
  emit objectCreated(obj);
  return obj;
}


KInstance *GccCompilerFactory::s_instance = 0;
KInstance *GccCompilerFactory::instance(){
  if (!s_instance)
    s_instance = new KInstance("KDevCompilerGcc");

  return s_instance;
}
#include "main.moc"
