/* This file is part of the KDE libraries
   Copyright (C) 2003 Joseph Wenninger

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KMDIGUICLIENT_H
#define KMDIGUICLIENT_H

#include <qobject.h>
#include <qguardedptr.h>
#include <kxmlguiclient.h>
#include <kaction.h>
#include "kmdidefines.h"

class KMainWindow;
class KToolBar;
class KMdiToolViewAccessor;
class KMdiMainFrm;
class KDockWidget;

namespace KMDIPrivate {

class KMDIGUIClient : public QObject,
                       public KXMLGUIClient
{
    Q_OBJECT
public:
    KMDIGUIClient( KMdiMainFrm *mdiMainFrm, bool showMDIModeAction, const char *name = 0 );
    virtual ~KMDIGUIClient();

    void addToolView(KMdiToolViewAccessor*);

private slots:
    void clientAdded( KXMLGUIClient *client );
    void setupActions();
    void changeViewMode(int id);
    void actionDeleted(QObject*);
    void mdiModeHasBeenChangedTo(KMdi::MdiMode);
signals:
    void toggleTop();
    void toggleLeft();
    void toggleRight();
    void toggleBottom();

private:
    class KMDIGUIClientPrivate;
    KMDIGUIClientPrivate *d;
    KMdi::MdiMode m_mdiMode;

    QGuardedPtr<KMdiMainFrm> m_mdiMainFrm;
    QPtrList<KAction> m_toolViewActions;
    QPtrList<KAction> m_documentViewActions;

    KActionMenu *m_docMenu;
    KActionMenu *m_toolMenu;
    KSelectAction *m_mdiModeAction;

    KActionMenu *m_gotoToolDockMenu;
};


class ToggleToolViewAction:public KToggleAction
{
Q_OBJECT
public:
        ToggleToolViewAction( const QString& text, const KShortcut& cut = KShortcut(),KDockWidget *dw=0,KMdiMainFrm *mdiMainFrm=0,
		QObject* parent = 0, const char* name = 0 );

        virtual ~ToggleToolViewAction();

private:
        KDockWidget *m_dw;
        KMdiMainFrm *m_mdiMainFrm;
protected slots:
        void slotToggled(bool);
        void anDWChanged();
        void slotWidgetDestroyed();
};


}

#endif
