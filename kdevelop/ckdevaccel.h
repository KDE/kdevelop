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

// general structure to the new CKDevAccel class
struct SActionInfo
{
	QString m_sAction;
	bool	m_bEnabled;

	SActionInfo::SActionInfo(const QString& sAction, bool bEnabled=true) :
		 m_sAction(sAction), m_bEnabled(bEnabled) {};

	SActionInfo::SActionInfo(const SActionInfo &Info) :
		 m_sAction(Info.m_sAction), m_bEnabled(Info.m_bEnabled) {};

};

/**  New accelaration class
     with reference to menu-ID
  *@author W. Tasin
  */

class CKDevAccel : public KAccel  {
  QIntDict<SActionInfo> m_Assoc;

public: 
	CKDevAccel( QWidget * parent, const char *name = 0 );
	~CKDevAccel();

protected:

public:
  // get the action string according to a resource ID
  //   if there's no entry, return an empty string
  QString getActionString(uint uMenuId) const;
  // get the resource ID according to the action
  //   if there is no entry, return 0
  uint getMenuID(const QString &action) const;

  // enables/disables all key accelerators according
  //  to the menu
  //  this will change entries only if
  //  the resource ID is present in the menu, otherwise
  //  the according accelerator will be unchanged
  void setEnableByMenuItems(const QMenuData *menu);

  // reimplementation of KAccel methods
  //  these methods allow you to bind an accelerator with
  //  a resource ID
  void connectItem(const QString& action,
    const QObject* receiver, const char *member,
    bool activate = true, uint uMenuId=0);

  void connectItem(KAccel::StdAccel accel,
    const QObject* receiver, const char *member,
    bool activate = true, uint uMenuId=0);

  void disconnectItem(const QString& action,
    const QObject* receiver, const char *member);

  // reconnect does the same as connect
  //   only the enable state of the accelerator will
  //   be untouched.
  void reconnectItem(const QString& action,
    const QObject* receiver, const char *member);

  void reconnectItem(KAccel::StdAccel accel,
    const QObject* receiver, const char *member);

  // like reconnect, only by resource ID
  //   instead of action string or StdAccel number
  bool reconnectItemByID(uint uMenuId,
    const QObject* receiver, const char *member);

  // reimplementation of KAccel::readSettings
  // the additional parameter sets the enable structure of
  //   each item in CKDevAccel to the saved value
  void readSettings (KConfig* config = 0, bool setEnableStruct=true);

  // reimplementation of KAccel::setItemEnabled
  //   enables or disables a certain accelerator key
  //   by using either the resource ID or the action string
  bool setItemEnabled( uint uMenuId, bool activate );
  void setItemEnabled( const QString& action, bool activate );

};

#endif

