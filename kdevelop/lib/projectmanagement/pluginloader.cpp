/***************************************************************************
                          pluginloader.cpp  -  description
                             -------------------
    begin                : Sat Jun 3 2000
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

#include "pluginloader.h"
#include "projectspace.h"
#include "appwizard.h"
#include "project.h"

#include <kstddirs.h>
#include <dlfcn.h>
#include <iostream.h>
#include <ksimpleconfig.h>
#include <qfileinfo.h>
#include <klibloader.h>
#include <kdebug.h>
#include <ktrader.h>



PluginLoader::PluginLoader(){
}
PluginLoader::~PluginLoader(){
}
/** returns a new projectspace, based on the pluginname from the configfile*/
ProjectSpace* PluginLoader::getNewProjectSpaceFromConfigfile(QString filename){
  cerr << "\nPluginLoader::getNewProjectSpaceFromConfigfile\n";
  KStandardDirs* std_dirs = KGlobal::dirs();
  // load the config
  KSimpleConfig conf(filename,true); // read only
  conf.setGroup("General");
  QString library_name = conf.readEntry("plugin_libraryname");
  
  QString lib =  std_dirs->findResource("lib",library_name);
  return PluginLoader::getNewProjectSpace(lib);
}

/** returns a new projectspace, based on the filename for the lib*/
ProjectSpace* PluginLoader::getNewProjectSpace(QString library_filename){
  void *handle=0;
  char* error;
  QObjectPlugin* (*create_function)( void);
  
  // load the projectspace plugin
  handle = dlopen( library_filename, RTLD_NOW ); // load the projectspace
  if (!handle ) {
    cerr << "\n" << handle <<"\ndlopen-error: " << dlerror() << endl;
  }
  else {
    create_function = (QObjectPlugin* (*)(void)) dlsym( handle, "create" ) ;
    
    if ( (error = dlerror()) == NULL ) {
      return (ProjectSpace*) (*create_function) () ;
    }
  } // end else
  return 0;
}
AppWizard* PluginLoader::getNewAppwizardPlugin(QString library_filename){
  void *handle=0;
  char* error;
  Plugin* (*create_function)( void);
  
  handle = dlopen(library_filename , RTLD_NOW ); // load the plugin
  if (!handle ) {
    cerr << "\n" << handle <<"\ndlopen-error: " << dlerror() << endl;
  }
  else {
    create_function = (Plugin* (*)(void)) dlsym( handle, "create" ) ;
    
    if ( (error = dlerror()) == NULL ) {
      return (AppWizard*) (*create_function) ();
    }
  } // end else
  return 0;
}

Project* PluginLoader::getNewProject(QString projecttype_name,QObject* parent){
  kdDebug(9000) << "enter PluginLoader::getNewProject";
  QString constraint = QString("[Name] == '%1'").arg(projecttype_name);
  KTrader::OfferList offers = KTrader::self()->query("KDevelop/Project", constraint);
  KService *service = *offers.begin();
  kdDebug(9000) << "Found Project Component " << service->name() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());
  if (!factory){
    kdDebug(9000) << "Factory not available " << service->library()  << endl;
  }
  
  Project* prj  = (Project*)factory->create(parent,service->name().latin1(),
					    "Project");
  if(!prj){
    kdDebug(9000) << "couldn't create the project "<<  service->library()  << endl;
  }
  kdDebug(9000) << "create new Project " << prj->getPluginName() << endl;
  return prj;
}

QString PluginLoader::findLibrary(QString library_name){		
  // get all ProjectSpaces
  KStandardDirs* std_dirs = KGlobal::dirs();
  QStringList::Iterator it;
  QStringList list = std_dirs->resourceDirs ("lib");
  for(it = list.begin();it != list.end();it++){
    cerr << *it << "\n";
  }
  
  QString lib = std_dirs->findResource("lib", library_name);
  return lib;
}

QList<ProjectSpace>* PluginLoader::getAllProjectSpaces(QObject* parent){
  // get all ProjectSpaces
  QStringList plugin_list;
  KStandardDirs* std_dirs = KGlobal::dirs();
  QList<ProjectSpace>* list = new QList<ProjectSpace>;
  QStringList::Iterator it;
  ProjectSpace* space;
  
  plugin_list =  std_dirs->findAllResources("lib", "*_projectspace.so");
  
  for(it = plugin_list.begin();it != plugin_list.end();it++){
    QFileInfo fi( *it );
    KLibFactory *factory = KLibLoader::self()->factory(fi.baseName());
    if (!factory){
      kdDebug(9000) << "Factory not available " << *it << endl;
    }
  
    space = (ProjectSpace*)factory->create(parent);
    if(space !=0){
      list->append(space);	
      kdDebug(9000) << "\nappend Projectspace\n" << space->getPluginName() << endl;
    }
  } // end for
  return list;
}

QList<AppWizard>* PluginLoader::getAllAppWizards(QObject* parent){
  // get all AppWizards
  QStringList plugin_list;
  KStandardDirs* std_dirs = KGlobal::dirs();
  QList<AppWizard>* list = new QList<AppWizard>;
  QStringList::Iterator it;
  AppWizard* wizard;
  
  plugin_list =  std_dirs->findAllResources("lib", "*_appwizard.so");
  
  for(it = plugin_list.begin();it != plugin_list.end();it++){
    QFileInfo fi( *it );
    KLibFactory *factory = KLibLoader::self()->factory(fi.baseName());
    if (!factory){
      kdDebug(9000) << "Factory not available " << *it << endl;
    }
  
    wizard = (AppWizard*)factory->create(parent);
    if(wizard !=0){
      list->append(wizard);	
      kdDebug(9000) << "\nappend AppWizard\n" << wizard->getPluginName() << endl;
    }
  } // end for
  return list;
}
