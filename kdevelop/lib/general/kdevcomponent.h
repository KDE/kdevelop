/***************************************************************************
                          kdevcomponent.h  -  base class for all components
                             -------------------                                         
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef _KDEVCOMPONENT_H_
#define _KDEVCOMPONENT_H_

#include <qstring.h>
#include <kparts/part.h>
#include <qdom.h>
#include <kaction.h>
#include <kdevelopcorebase.h>
#include <qlist.h>

class KDevVersionControl;
class KDevLanguageSupport;
class KDevEditorManager;
class KDevMakeFrontend;
class KDevAppFrontend;
class ProjectSpace;
class Project;
class ClassStore;
class KDialogBase;
class KAboutData;
class KDevNode;
class KDevFileNode;
class KDevApi;


/**
 * This is the base class for all components like doc tree view, LFV, RFV
 * and so on. It contains a collection of virtual classes which components
 * can override if they want to be informed about several kinds of 'events'
 * happening. In this way, it is possible to move much code from the class
 * CKDevelop which handles special cases into separate modules. And of
 * course, this functionality is essential for non-trivial plugins.
 */

class KDevComponent : public KParts::Part
{
    Q_OBJECT
    
public:
    enum Role { SelectView, OutputView, DocumentView };
    
    KDevComponent( QObject *parent=0, const char *name=0 );
    ~KDevComponent();

    /**
     * Returns a pointer to the KDevelop Core. This will enable components
       to traverse the component list and raise events which can cascade to
       all other components.
     */
    KDevelopCoreBase *TheCore();
    /**
     * Gives a reference to the current project space component
     */
    ProjectSpace *projectSpace();
    /**
     * Gives a reference to the class store
     */
    ClassStore *classStore();
    /**
     * Gives a reference to the version control component
     */
    KDevVersionControl *versionControl();
    /**
     * Gives a reference to the language support component
     */
    KDevLanguageSupport *languageSupport();
    /**
     * Gives a reference to the editor manager component
     */
    KDevEditorManager *editorManager();
    /**
     * Gives a reference to the make frontend component
     */
    KDevMakeFrontend *makeFrontend();
    /**
     * Gives a reference to the application frontend component
     */
    KDevAppFrontend *appFrontend();
    /**
     * Setup the component with its GUI. Note that the component's
     * widgets must be created here, not in the constructor.
     */
    virtual void setupGUI();
    /**
     * Creates a configuration page for use in the
     * KDevelop settings dialog.
     */
    virtual void configWidgetRequested(KDialogBase *dlg);
    /**
     * The user has clicked the stop button.
     */
    virtual void stopButtonClicked();
    /**
     * A projectspace has been opened.
     * This occasion may e.g. be used to read ProjectSpace related
     * configuration from the document. User independent data is
     * available in the DOM document projectSpace->readUserDocument().
     * User dependent data (it is important for multiuser projects
     * to distinguish them) are available under projectSpace->readGlobalDocument().
     */
    virtual void projectSpaceOpened();

    /** The current ProjectSpace is in the process of closing.
    */
    virtual void projectSpaceClosed();

    /** When the user switches to a different sub project in their projectspace
        we will call this routine for each component.
     */
    virtual void projectChanged(){};

    /** returns some data about this Component, should be static?*/
    virtual KAboutData* aboutPlugin();
  
    /**
     * return a list of KDevFileActions which are implemented by this component for this KDevNode
     */
    virtual QList<KAction> kdevNodeActions(KDevNode* pNode);
  
signals:
    void embedWidget(QWidget *, KDevComponent::Role, const QString&, const QString&);
    /**
     * Marks the component as running (or not running). As long as at least one
     * component is running, the stop button is enabled. When it is pressed,
     * all components get a stopButtonClicked().
     */
    void running(bool runs);
    /**
     * Gets all Actions for this file from every component,
     * assembled with QList<KAction> kdevNodeActions(KDevNode* pNode)
     */
    void needKDevNodeActions(KDevNode* pNode, QList<KAction> *pList);

private:
    // These methods are not meaningful and are therefore forbidden
    void setWidget(QWidget *w);
    QWidget *widget();

    // Internal
    friend class KDevelopCore;
    void setupInternal (KDevelopCoreBase * pCore, KDevApi *api);
    KDevApi             *m_api;
    KDevelopCoreBase    *m_pCore;       // Points to the KDevelop core.
};

#endif
