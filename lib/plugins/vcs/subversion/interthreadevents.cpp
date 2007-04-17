#include "interthreadevents.h"

SvnNotificationEvent::SvnNotificationEvent( QString msg )
    : QEvent( SVNACTION_NOTIFICATION ), m_msg(msg)
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

void SvnLoginInfo::setData( QString userId, QString passWd, bool save )
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

void SvnCommitLogInfo::setData( bool accept, QString msg )
{
    m_accept = accept;
    m_message = msg;
    setHasInfos();
}

