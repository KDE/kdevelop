/***************************************************************************
                          ckdevaccel.h  -  description
                             -------------------
    begin                : Thu Sep 23 1999
    copyright            : (C) 1999 by W. Tasin
    email                : tasin@e-technik.fh-muenchen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CKDEVACCEL_H
#define CKDEVACCEL_H

#include <qobject.h>
#include <qstring.h>
#include <qintdict.h>
#include <kaccel.h>

/**  New accelaration class
     with reference to menu-ID
  *@author W. Tasin
  */

class CKDevAccel : public KAccel  {
  QIntDict<QString> assoc;

public: 
	CKDevAccel( QWidget * parent, const char *name = 0 );
	~CKDevAccel();

  void connectItem(const QString& action,
	  const QObject* receiver, const char *member,
    bool activate = true, uint uMenuId=0);

  void connectItem(KAccel::StdAccel accel,
	  const QObject* receiver, const char *member,
    bool activate = true, uint uMenuId=0);

  bool setItemEnabled( uint uMenuId, bool activate );
  void setItemEnabled( const QString& action, bool activate );

};

#endif

