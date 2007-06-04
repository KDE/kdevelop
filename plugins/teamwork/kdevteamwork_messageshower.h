/***************************************************************************
  copyright            : (C) 2006 by David Nolden
  email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MESSAGESHOWER_H
#define MESSAGESHOWER_H

class QWidget;
class KDevTeamworkTextMessage;
class KDevTeamwork;
class KDevTeamworkUser;
class InDocumentMessage;

#include <QObject>
#include "ui_kdevteamwork_textmessage.h"
#include "ui_kdevteamwork_userinfo.h"
#include <QPointer>
#include "network/safesharedptr.h"
#include "kdevteamwork_helpers.h"
#include "safelogger.h"
#include "autoconstructpointer.h"


class MessageSerialization;

class InDocumentMessageShowerData;

namespace Teamwork {
class MessageInterface;
typedef SafeSharedPtr<MessageInterface, MessageSerialization> MessagePointer;
}

using namespace Teamwork;

class KDevTeamworkMessageShower : public QObject {
    Q_OBJECT
  public:
    KDevTeamworkMessageShower( KDevTeamworkTextMessage* msg, QWidget* widget, KDevTeamwork* teamwork );
  public slots:
    virtual void isReplyToButton();
    virtual void menuButton();
    virtual void historyButton();
    virtual void userStateChanged();
    virtual void messageUpdated( const MessagePointer& msg );
  protected:
    SafeSharedPtr<KDevTeamworkTextMessage> m_message;
    QWidget* m_widget;
    Ui_TextMessageShower m_widgetData;
    QPointer<KDevTeamwork> m_teamwork;

    virtual void fillData();

  private:
    void fill();
};


class KDevTeamworkUserInfoShower : public QObject {
    Q_OBJECT
  public:
    KDevTeamworkUserInfoShower( KDevTeamworkUser* user, QWidget* widget, KDevTeamwork* teamwork );
  public slots:
    virtual void menuButton();
    virtual void historyButton();
    virtual void userStateChanged();
  private:
    void setIp();
    void setStatus();
    void setIcon();

    SafeSharedPtr< KDevTeamworkUser > m_user;
    QWidget* m_widget;
    Ui_UserInfo m_widgetData;
    QPointer<KDevTeamwork> m_teamwork;
};

class InDocumentMessageShower : public KDevTeamworkMessageShower, private SafeLogger {
    Q_OBJECT
  public:
    InDocumentMessageShower( InDocumentMessage* msg, QWidget* widget, KDevTeamwork* teamwork );
  public slots:
    void fileClicked();
    void contextClicked();
  protected:

    virtual void fillData();
  private:
    void fill();
    AutoConstructPointer<InDocumentMessageShowerData> m_data;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
