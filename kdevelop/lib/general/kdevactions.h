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


class KDevFileAction : public KAction {
  Q_OBJECT
public:
  KDevFileAction( const QString& text, const QString& pix, int accel = 0, QObject* parent = 0, const char* name = 0 );
  KDevFileAction( const QString& text,int accel = 0, QObject* parent = 0, const char* name = 0 );
  
  int plug( QWidget *widget, int index );
  void setAbsFileName(QString absFileName);
  void setProjectName(QString projectName);
  
  virtual ~KDevFileAction() {}

protected slots:
virtual void slotActivated();

  signals:
/** connect to this*/
void activated(const QString& absFileName,const QString& projectName);
  
private:
  QString m_absFileName;
  QString m_projectName;
};
#endif
