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
#include <qpointer.h>
//Added by qt3to4:
#include <Q3PtrList>
#include <kxmlguiclient.h>
#include <kaction.h>
#include "kmdidefines.h"

class KMainWindow;
class KToolBar;
class KMdiToolViewAccessor;
class KMdiMainFrm;
class KDockWidget;

namespace KMDIPrivate {

/**
 * A class derived from KXMLGUIClient that handles the various
 * KMDI modes
 */
class KMDI_EXPORT KMDIGUIClient : public QObject,
                       public KXMLGUIClient
{
    Q_OBJECT
public:

    KMDIGUIClient( KMdiMainFrm *mdiMainFrm, bool showMDIModeAction, const char *name = 0 );
    virtual ~KMDIGUIClient();

    /**
     * Add a new tool view to this KMDIGUIClient. Reads the shortcut
     * for the tool view from the KMDI application's config file and also
     * adds a ToggleToolViewAction so that the visibility of the toolviews
     * can be turned on and off
     */
    void addToolView(KMdiToolViewAccessor*);

private slots:

    /**
     * The XMLGUIClient factory has added an XMLGUI client. Plug our actions
     * in if we're the client that's been added.
     */
    void clientAdded( KXMLGUIClient *client );
    /**
     * Plug in the various toggle actions we have into the tool views menu
     */
    void setupActions();

    /**
     * Change the view mode. This will automatically change the view mode
     * of the KMdiMainFrm associated with this KMDIGUIClient
     */
    void changeViewMode(int id);

    /**
     * One of our tool view toggle actions has been deleted. Redo the
     * tool views menu
     */
    void actionDeleted(QObject*);

    /**
     * Updates the action that lets the user change the MDI mode to the
     * correct value based on the current mode
     */
    void mdiModeHasBeenChangedTo(KMdi::MdiMode);

signals:
    /** Toggle the top tool dock */
    void toggleTop();

    /** Toggle the left tool dock */
    void toggleLeft();

    /** Toggle the right tool dock */
    void toggleRight();

    /** Toggle the bottom tool dock */
    void toggleBottom();

private:
    class KMDIGUIClientPrivate;
    KMDIGUIClientPrivate *d;
    KMdi::MdiMode m_mdiMode;

    QPointer<KMdiMainFrm> m_mdiMainFrm;
    Q3PtrList<KAction> m_toolViewActions;
    Q3PtrList<KAction> m_documentViewActions;

    KActionMenu *m_docMenu;
    KActionMenu *m_toolMenu;
    KSelectAction *m_mdiModeAction;

    KActionMenu *m_gotoToolDockMenu;
};

/**
 * A KToggleAction specifically for toggling the showing
 * or the hiding of a KMDI tool view
 */
class KMDI_EXPORT ToggleToolViewAction:public KToggleAction
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
