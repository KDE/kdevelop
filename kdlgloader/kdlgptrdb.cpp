/***************************************************************************
                          kdlgptrdb.cpp  -  description                              
                             -------------------                                         
    begin                : Fri Jun 4 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer                         
    email                : pascal@beast.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "kdlgptrdb.h"
#include "kdlgldr.h"

KDlgPtrDb::KDlgPtrDb(KDlgPtrDb *parent, QWidget *ch )
{
#ifdef dlgldr_wth
  char msg[255];
  sprintf(msg, "[KDlgPtrDb::KDlgPtrDb()] this=%X parent=%X", (unsigned int)this, (unsigned int)parent);
  WatchMsg(msg);
#endif

  Parent = parent;
  Name = "";

//  childs.setAutoDelete( true );
//  childs.clear();
  groups.setAutoDelete( true );
  groups.clear();
  child = ch;
}


KDlgPtrDb::~KDlgPtrDb()
{
#ifdef dlgldr_wth
  WatchMsg("[KDlgPtrDb::~KDlgPtrDb()]");
#endif

  deleteMyself();
}


void KDlgPtrDb::deleteMyself()
{
#ifdef dlgldr_wth
  WatchMsg("[KDlgPtrDb::deleteMyself()]");
#endif
//  childs.clear();
  groups.clear();
}


void KDlgPtrDb::setName(QString n)
{
#ifdef dlgldr_wth
  char msg[255];
  sprintf(msg, "[KDlgPtrDb::setName(\"%s\")] this=%X", (const char*)n, (unsigned int)this);
  WatchMsg(msg);
#endif

  Name = n;
}


void KDlgPtrDb::setVarName(QString n)
{
#ifdef dlgldr_wth
  char msg[255];
  sprintf(msg, "[KDlgPtrDb::setVarName(\"%s\")] this=%X", (const char*)n, (unsigned int)this);
  WatchMsg(msg);
#endif

  VarName = n;
}

  	
QWidget* KDlgPtrDb::getItemPtrVarName(QString itemName)
{
  if (itemName.isEmpty())
    return 0;

  if (VarName.lower() == itemName)
    {
      return child;
    }

  QWidget *res=0;
  KDlgPtrDb* d;
  for (d=groups.first(); d!=0; d=groups.next())
    {
      res = d->getItemPtrVarName(itemName);
      if (res)
        {
          return res;
        }
    }

  return 0;
}

QWidget* KDlgPtrDb::getItemPtrName(QString itemName)
{
  if (itemName.isEmpty())
    return 0;

  if (Name.lower() == itemName)
    {
      return child;
    }

  QWidget *res=0;
  KDlgPtrDb* d;
  for (d=groups.first(); d!=0; d=groups.next())
    {
      res = d->getItemPtrName(itemName);
      if (res)
        {
          return res;
        }
    }

  return 0;
}






