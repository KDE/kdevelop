/***************************************************************************
                          ckdevaccel.cpp  -  description
                             -------------------
    begin                : Thu Sep 23 1999
    copyright            : (C) 1999 by The KDevelop Team
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ckdevaccel.h"

CKDevAccel::CKDevAccel( QWidget * parent, const char *name )
 : KAccel ( parent, name )
{
  assoc.setAutoDelete(true);
}

CKDevAccel::~CKDevAccel()
{
  assoc.clear();
}

void CKDevAccel::connectItem(const QString& action,
	const QObject* receiver, const char *member, bool activate, uint uMenuId)
{

  if (uMenuId!=0)
  {
   if ( assoc.find(uMenuId) )
       assoc.remove( uMenuId );
   assoc.insert( uMenuId, new QString(action) );
  }

  KAccel::connectItem(action, receiver, member, activate);
}

void CKDevAccel::connectItem(KAccel::StdAccel accel,
	const QObject* receiver, const char *member, bool activate, uint uMenuId)
{
  if (uMenuId!=0)
  {
   if ( assoc.find(uMenuId) )
       assoc.remove( uMenuId );
   assoc.insert( uMenuId, new QString(KAccel::stdAction(accel)) );
  }

  KAccel::connectItem(accel, receiver, member, activate);
}

/**
    like KAccel::setItemEnabled()
    only the menu ID will be used to determine which
    accelerator is to disable

    returns true if an associated key-accel was found
*/
bool CKDevAccel::setItemEnabled( uint uMenuId, bool activate )
{
  bool bRetVal=false;
  if ( assoc.find(uMenuId) )
  {
    KAccel::setItemEnabled(*(assoc[uMenuId]), activate);
    bRetVal=true;
  }

  return bRetVal;
}

void CKDevAccel::setItemEnabled( const QString &action, bool activate )
{
    KAccel::setItemEnabled(action, activate);
}
