/***************************************************************************
*   Copyright (C) 2004 by ian geiser                                      *
*   geiseri@sourcextreme.com                                              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef KScriptAction_H
#define KScriptAction_H

#include <scriptclientinterface.h>
#include <qobject.h>
#include <qptrlist.h>

class KAction;
class KActionCollection;
class KScriptInterface;



/**
* Connects a KAction to a script runner.
* 
* @author ian geiser geiseri@sourcextreme.com
*/
class KScriptAction : public QObject, public KScriptClientInterface {
    Q_OBJECT
public:

    KScriptAction( const QString &scriptDesktopFile, QObject *parent, KActionCollection *ac );

    virtual ~KScriptAction();

    /**
    * KAction for the current script
     */
    KAction *action();

    /**
    * Returns the validity of the current script.
     */
    bool isValid() const;

    // Reimplemented from KScript
    void error ( const QString &msg );
    void warning ( const QString &msg );
    void output ( const QString &msg );
    void progress ( int percent );
    void done ( KScriptClientInterface::Result result, const QVariant &returned );

signals:
    void scriptError ( const QString &msg );
    void scriptWarning ( const QString &msg );
    void scriptOutput ( const QString &msg );
    void scriptProgress ( int percent );
    void scriptDone ( KScriptClientInterface::Result result, const QVariant &returned );

private slots:
    void activate();

private:
    KAction *m_action;
    QString m_scriptName;
    QString m_scriptType;
    QString m_scriptFile;
    QString m_scriptMethod;
    KScriptInterface *m_interface;
};

/**
 * Provides an actionlist of scripts that are currently available.
 * Scripts are not actually loaded until they are actually executed.
 * @author ian geiser <geiseri@sourcextreme.com>
 */
class KScriptActionManager {
public:
    /**
    * Create a script manager that is attached to an action collection.
     */
    KScriptActionManager( KActionCollection *ac );
    ~KScriptActionManager();

    /**
    * Return all currently loaded scripts in a direcotry and attaches them
    * to a QObject interface.  If the dirs are empty the current applications
    * scripts directory is used.  The dirs are actual directories to search
    * in the $KDEPATH/data/ direcories.  So if you add "coolapp/data" then
    * the manager will search in $KDEPATH/data/coolapp/data for all desktop
    * files that are scripts.
     */
    QPtrList<KAction> scripts( QObject *interface, const QStringList &dirs = QStringList() ) const;

private:
    mutable QPtrList<KScriptAction> m_actions;
    KActionCollection *m_ac;
};
#endif
