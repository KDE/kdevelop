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
  m_Assoc.setAutoDelete(true);
}

CKDevAccel::~CKDevAccel()
{
  m_Assoc.clear();
}

QString CKDevAccel::getActionString(uint uMenuId) const
{
   QString action;
   if ( m_Assoc.find(uMenuId) )
       action=m_Assoc[uMenuId]->m_sAction;

   return action;
}

uint CKDevAccel::getMenuID(const QString &action) const
{
  QIntDictIterator<SActionInfo> it(m_Assoc);
  uint uMenuId=0;

  if (!action.isEmpty())
  {
    while ( it.current() && uMenuId==0)
    {
      if (it.current()->m_sAction == action)
        uMenuId=it.currentKey();
      ++it;
    }
  }

  return uMenuId;
}

void CKDevAccel::setEnableByMenuItems(const QMenuData *menu)
{
  QIntDictIterator<SActionInfo> it(m_Assoc);

  while ( it.current() )
  {
    if (menu->indexOf(it.currentKey()) != -1)
      setItemEnabled(it.currentKey(), menu->isItemEnabled(it.currentKey()));
    ++it;
  }

}

void CKDevAccel::connectItem(const QString& action,
	const QObject* receiver, const char *member, bool activate, uint uMenuId)
{

  if (uMenuId!=0)
  {
   if ( m_Assoc.find(uMenuId) )
       m_Assoc.remove( uMenuId );
   m_Assoc.insert( uMenuId, new SActionInfo(action, activate) );
  }

  KAccel::connectItem(action, receiver, member, activate);
}

void CKDevAccel::connectItem(KAccel::StdAccel accel,
	const QObject* receiver, const char *member, bool activate, uint uMenuId)
{
  if (uMenuId!=0)
  {
   if ( m_Assoc.find(uMenuId) )
       m_Assoc.remove( uMenuId );
   m_Assoc.insert( uMenuId, new SActionInfo(KAccel::stdAction(accel), activate) );
  }

  KAccel::connectItem(accel, receiver, member, activate);
}

void CKDevAccel::disconnectItem(const QString& action,
	const QObject* receiver, const char *member)
{
  int uMenuId=getMenuID(action);
  if (uMenuId!=0)
       m_Assoc.remove( uMenuId );

  KAccel::disconnectItem(action, receiver, member);
}

void CKDevAccel::reconnectItem(const QString& action,
	const QObject* receiver, const char *member)
{
  uint uMenuId=getMenuID(action);
  bool activate=true;

  if (uMenuId!=0)
    activate=m_Assoc[uMenuId]->m_bEnabled;

  KAccel::connectItem(action, receiver, member, activate);
}

void CKDevAccel::reconnectItem(KAccel::StdAccel accel,
	const QObject* receiver, const char *member)
{
  uint uMenuId=getMenuID(KAccel::stdAction(accel));
  bool activate=true;

  if (uMenuId!=0)
    activate=m_Assoc[uMenuId]->m_bEnabled;

  KAccel::connectItem(accel, receiver, member, activate);
}

bool CKDevAccel::reconnectItemByID(uint uMenuId,
	const QObject* receiver, const char *member)
{
  bool bFound=false;
  if ( m_Assoc.find(uMenuId) )
  {
       bool activated=m_Assoc[uMenuId]->m_bEnabled;
       QString action=m_Assoc[uMenuId]->m_sAction;

       KAccel::connectItem(action, receiver, member, activated);
       bFound=true;
  }
  return bFound;
}

void CKDevAccel::readSettings(KConfig* config, bool setEnableStruct)
{
   KAccel::readSettings(config);
   if (setEnableStruct)
   {
     QIntDictIterator<SActionInfo> it(m_Assoc);
     while ( it.current())
     {
       QString action=it.current()->m_sAction;
       if (!action.isEmpty())
        KAccel::setItemEnabled(action, it.current()->m_bEnabled);
       ++it;
     }

   }
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
  if ( m_Assoc.find(uMenuId) )
  {
    m_Assoc[uMenuId]->m_bEnabled=activate;
    KAccel::setItemEnabled(m_Assoc[uMenuId]->m_sAction, activate);
    bRetVal=true;
  }

  return bRetVal;
}

void CKDevAccel::setItemEnabled( const QString &action, bool activate )
{
    uint uMenuId=getMenuID(action);
    if (uMenuId != 0)
      m_Assoc[uMenuId]->m_bEnabled=activate;

    KAccel::setItemEnabled(action, activate);
}
