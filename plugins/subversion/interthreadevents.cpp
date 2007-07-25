/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "interthreadevents.h"

SvnNotificationEvent::SvnNotificationEvent( const QString &path, const QString& msg )
    : QEvent( SVNACTION_NOTIFICATION ), m_path(path), m_msg(msg)
{}

SvnNotificationEvent::~SvnNotificationEvent()
{
}

SvnInterThreadPromptEvent::SvnInterThreadPromptEvent(QEvent::Type type, void *data)
    : QEvent(type), m_data(data)
{}

SvnInterThreadPromptEvent::~SvnInterThreadPromptEvent()
{}

SvnUserinputInfo::SvnUserinputInfo()
{
     hasInfo = false;
}
bool SvnUserinputInfo::receivedInfos()
{
     return hasInfo;
}

void SvnUserinputInfo::setHasInfos()
{
     hasInfo = true;
}

SvnLoginInfo::SvnLoginInfo()
    : SvnUserinputInfo()
{}

void SvnLoginInfo::setData( const QString& userId, const QString& passWd, bool save )
{
    userName = userId;
    passWord = passWd;
    maySave = save;
    setHasInfos();
}

SvnServerCertInfo::SvnServerCertInfo()
    : SvnUserinputInfo()
{}

void SvnServerCertInfo::setData( int decision )
{
    m_decision = decision;
    setHasInfos();
}

SvnCommitLogInfo::SvnCommitLogInfo()
    : SvnUserinputInfo()
{}

void SvnCommitLogInfo::setData( bool accept, const QString& msg )
{
    m_accept = accept;
    m_message = msg;
    setHasInfos();
}

