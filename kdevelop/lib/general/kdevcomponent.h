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
#include <qlist.h>

class KDevVersionControl;
class KDevLanguageSupport;
class ProjectSpace;
class ClassStore;
class KDialogBase;
class KAboutData;
class KDevNode;
class KDevFileNode;



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
    enum Role { SelectView, OutputView, AreaOfDocumentViews, DocumentView };
    
    KDevComponent( QObject *parent=0, const char *name=0 );
    ~KDevComponent();

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
     * Starts a compilation or any other command.
     */
    virtual void commandRequested(const QString &command);
    /**
     * The user has clicked the stop button.
     */
    virtual void stopButtonClicked();
    /**
     * A projectspace  has been opened.
     */
    virtual void projectSpaceOpened(ProjectSpace *projectSpace);
    /**
     * The projectspace has been closed.
     */
    virtual void projectSpaceClosed();
    /**
     * A new version control version has been created.
     */
    virtual void versionControlOpened(KDevVersionControl *vc);
    /**
     * The version control object has been deleted.
     */
    virtual void versionControlClosed();
    /**
     * A new language support object has been created.
     */
    virtual void languageSupportOpened(KDevLanguageSupport *ls);
    /**
     * The language support object has been deleted.
     */
    virtual void languageSupportClosed();
    virtual void classStoreOpened(ClassStore *store);
    virtual void classStoreClosed();
    /**
     * A file has been added to the project
     */
    virtual void addedFileToProject(KDevFileNode* pNode);
    /**
     * A file has been removed from the project. 
     */
    virtual void removedFileFromProject(KDevFileNode* pNode);
    /**
     * A file has been saved. The filename is
     * given relative to the project dir.
     */
    virtual void savedFile(const QString&);

    /** returns some data about this Component, should be static?*/
    virtual KAboutData* aboutPlugin();
  
  /** set the assembled Actions , asked in needKDevNodeActions(KDevNode),
   */
  virtual  void setKDevNodeActions(QList<KAction>* pActions);
  /**
     return a list of KDevFileActions which are implemented by this component for this KDevNode
  */
  virtual QList<KAction>* kdevNodeActions(KDevNode* pNode);
  

    /** write the ProjectSpace related configuration to the document
     , but only user independent data (important for multiuser projects)*/
    virtual void writeProjectSpaceGlobalConfig(QDomDocument& doc);

    /** write the ProjectSpace related configuration to the document
	, saves user dependent data */
    virtual void writeProjectSpaceUserConfig(QDomDocument& doc);
    
    /** read the ProjectSpace related configuration from the document
	, but only user independent data (important for multiuser projects)*/
    virtual void readProjectSpaceGlobalConfig(QDomDocument& doc);

    /** read the ProjectSpace related configuration from the document
	, read user dependent data */
    virtual void readProjectSpaceUserConfig(QDomDocument& doc);

signals:
    void embedWidget(QWidget *, KDevComponent::Role, const QString&, const QString&);
    /**
     * Executes a make-like command. The output of the command is put
     * in a separate view and is parsed for error messages.
     */
    void executeMakeCommand(const QString &command);
    /**
     * Executes a command to start the application. The output of the command
     * is put in a separate view (except for terminal applications, which
     * are started in their own terminal emulator)
     */
    void executeAppCommand(const QString &command);
    /**
     * Marks the component as running (or not running). As long as at least one
     * component is running, the stop button is enabled. When it is pressed,
     * all components get a stopButtonClicked().
     */
  void running(bool runs);
  /** get all Actions for this file from every component,
      set through setKDevNodeActions(QList<KDevNodeAction>* actions);
      assembled with QList<KAction>* kdevNodeActions(KDevNode* pNode)
      pWho is the sender
   */
  void needKDevNodeActions(KDevComponent* pWho,KDevNode* pNode);

    void gotoSourceFile(const QString &fileName, int lineNo);
    void gotoDocumentationFile(const QString &fileName);
    void gotoProjectApiDoc();
    void gotoProjectManual();
private:
    // These methods are not meaningful and are therefore forbidden
    void setWidget(QWidget *w);
    QWidget *widget();
};


#endif
