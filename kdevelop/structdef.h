/***************************************************************************
                     structdef.h -- some important struct definitions
                             -------------------                                         

    begin                : 20 Jul 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
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

#ifndef STRUCTDEF_H
#define STRUCTDEF_H

#include <qlist.h>
#include <qstring.h>
#include <qdatetime.h>

/** (struct) contains the internal representation of a textfile
  *@author Sandy Meier
  */
struct TEditInfo {
  /**  the id in the menu-buffer*/
  int id; 
  QString filename;
  QCString text; // should be removed
  bool modified; // modified ?
  int cursor_line;
  int cursor_col;
  QDateTime last_modified;
};

struct TBugReportInfo {
    QString email;
    QString author;
    QString kde_version;
    QString qt_version;
    QString os;
    QString compiler;
    QString sendmail_command;
};
#endif
