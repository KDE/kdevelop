/***************************************************************************
                          kdevplugin.h  -  
                             -------------------
    begin                : Thu Sep 22 1999
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

#ifndef KDEVPLUGIN_H
#define KDEVPLUGIN_H

#include <qpopupmenu.h>

#include "ckdevelop.h"

class KDevPlugin : public QObject {
    Q_OBJECT

public: 
  /** construtor */
  KDevPlugin(); 
  /** destructor */
  virtual ~KDevPlugin();
 
  virtual void start();
  virtual void stop();
  
  void init(TImportantPtrInfo* info);

  //plugin infos
  QString name;
  QString author;
  QString description;
  QString copyright;
  QString homepage;
  QString email;
  QString version;

 protected:
    CKDevelop* kdev;
  QPopupMenu* plugin_menu;

};
#endif
