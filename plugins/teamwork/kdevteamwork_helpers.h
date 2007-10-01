/***************************************************************************
Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVTEAMWORK_HELPERS
#define KDEVTEAMWORK_HELPERS

#include"teamworkfwd.h"
#include <QString>
#include <QIcon>
#include <QMap>
#include <QPair>
#include <QObject>
#include <kicontheme.h>
#include "network/logger.h"

///This file contains a few classes that I currently don't know where else to put

using namespace Teamwork;

///Since deleting the client by a reference-counter in some meta-object leads to crashes, this is used to keep the client alive longer and delete it as last.
class LaterDeleter : public QObject {
  Q_OBJECT
  public:
    LaterDeleter( const KDevTeamworkClientPointer& c );
    ~LaterDeleter();
    KDevTeamworkClientPointer m_c;
};

class SafeTeamworkEmitter : public QObject, public Shared {
    Q_OBJECT
  public:
    SafeTeamworkEmitter( KDevTeamwork* tw );

    void updateMessageInfo( const MessagePointer& msg );
  signals:
    void signalUpdateMessageInfo( MessagePointer );
};

typedef SharedPtr<SafeTeamworkEmitter> SafeTeamworkEmitterPointer;


class IconCache {
    typedef QMap<QPair<QString, KIconLoader::Group>, QIcon> IconMap;
    QMap<QString, QString> maps_;
    IconMap m_icons;
    static IconCache* m_instance;
  public:
    IconCache();

    QIcon operator () ( const QString& name, KIconLoader::Group grp = KIconLoader::Small );

    static QIcon getIconStatic( QString name, KIconLoader::Group grp = KIconLoader::Small );

    QIcon getIcon( QString name, KIconLoader::Group grp = KIconLoader::Small );

    static IconCache* instance();
};


class KDevTeamworkLogger : public QObject, public Teamwork::Logger {
    Q_OBJECT
    KDevTeamwork* m_teamwork;
  public:
    ///This can be overridden for custom logging
    KDevTeamworkLogger( KDevTeamwork* tw );

    virtual void log( const std::string& str , Level lv );

    void invalidate() {
      m_teamwork = 0;
    }

    ///This function is used when an error occurred while locking the Logger, or in other dangerous error-cases. It must be thread-safe.
    virtual void safeErrorLog( const std::string& str, Level lv );
};

/** This is a helper-class for easy logging
 *  Easiest way of using it: use it as a base-class in a class that should do logging,
 *  initialize it with the name of the class and the correct logger-object, and than start
 *  logging by streaming to out(..) or err(). ( Example: err() << "could not open file"; )
 *
 * */

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
