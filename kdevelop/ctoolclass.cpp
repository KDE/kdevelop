/***************************************************************************
                 ctoolclass.cpp - some methods,that would be normal global,but I
		              hate global function :-)

    begin                : 20 Jan 1999                        
    copyright            : (C) 1999 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                         
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "ctoolclass.h"
#include <qfile.h>
#include <kmessagebox.h>
#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <iostream>
using namespace std;

bool CToolClass::searchProgram(const QString& name, bool allowWarningMsg )
{
  if (findProgram(name).isEmpty())
  {
    if (allowWarningMsg)
    {
      KMessageBox::sorry(0,
                        i18n("KDevelop needs \"%1\" to work properly.\n\tPlease install it!").arg(name),
                        i18n("Program not Found!"));
    }
    return false;
  }

  return true;
}

QString CToolClass::findProgram(const QString& name)
{
  if (name.left(1) == "/" && QFile::exists(name))
    return name;

  QStringList paths;
  QString file="";
  QString complete_path = getenv("PATH");
  paths = QStringList::split ( ":", complete_path, FALSE );

  for ( QStringList::Iterator it = paths.begin(); it != paths.end(); ++it )
  {
    if (QFile::exists((*it) + "/" + name))
    {
      file = (*it) + "/" + name;
      break;
    }
  }

  return file;
}

QString CToolClass::getRelativePath(QString source_dir,QString dest_dir){
  // a special case , the dir are equals
  if (source_dir == dest_dir){
    return "./";
  }
  dest_dir.remove(0,1); // remove beginning /
  source_dir.remove(0,1); // remove beginning /
  bool found = true;
  int slash_pos=0;


  do {
    slash_pos = dest_dir.find('/');
    if (dest_dir.left(slash_pos) == source_dir.left(slash_pos)){
      dest_dir.remove(0,slash_pos+1);
      source_dir.remove(0,slash_pos+1);
    }
    else {
      found = false;
    }
  }
  while(found == true);

  int slashes = source_dir.contains('/');
  int i;
  for(i=0;i < slashes;i++){
    dest_dir.prepend("../");
  }

  return dest_dir;
}

void CToolClass::readToolConfig(ToolAppList& toolList)
{
  QStrList exeList, labelList, argList, outputList, paneList;
  KConfig* config = KGlobal::config();

  toolList.clear();

  config->setGroup("ToolsMenuEntries");
  config->readListEntry("Tools_exe",exeList);
  config->readListEntry("Tools_entry",labelList);
  config->readListEntry("Tools_argument",argList);
  config->readListEntry("Tools_output",outputList);
  config->readListEntry("Tools_newpane",paneList);

  for (uint i = 0; i < exeList.count(); ++i) {
    toolList.append( CToolApp(labelList.at(i), exeList.at(i), argList.at(i), (QString(outputList.at(i)).compare("1") == 0), (QString(paneList.at(i)).compare("1") == 0)) );
//    kdDebug() << "Tool: " << "\"" << labelList.at(i) << "\" \"" << exeList.at(i) << "\" \"" << argList.at(i) << "\" \"" << outputList.at(i) << "\" \"" << paneList.at(i) << "\"" << endl;
  }
}

void CToolClass::writeToolConfig(const ToolAppList& toolList)
{
  // We save each attribute in a StringList so downward compatibility with 1.4 isn't broken

  QStrList exeList, labelList, argList, outputList, paneList;

  ToolAppList::ConstIterator it;
  for (it = toolList.begin(); it != toolList.end(); ++it) {
    exeList.append( (*it).getExeName() );
    labelList.append( (*it).getLabel() );
    // make sure the arguments string is not empty or writeEntry(QStrList)
    // will not work as expected - add a space instead (whitespace will be ignored by the shell)
    argList.append( (*it).getArgs().isEmpty() ? QString(" ") : (*it).getArgs());    
    outputList.append( (*it).isOutputCaptured() ? "1" : "0" );
    paneList.append( (*it).isInNewPane() ? "1" : "0" );
  }

  KConfig* config = KGlobal::config();

  if (!config)
    return;

  config->setGroup("ToolsMenuEntries");
  config->writeEntry( "Tools_exe",exeList );
  config->writeEntry( "Tools_entry",labelList );
  config->writeEntry( "Tools_argument",argList );
  config->writeEntry( "Tools_output",outputList );
  config->writeEntry( "Tools_newpane",paneList );
}
