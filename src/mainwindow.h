/***************************************************************************
  kdevmainwindow.h - KDevelop main widget for all QextMDI-based user
                   interface modes (Childframe, TabPage, Toplevel)
                             -------------------
    begin                : 22 Dec 2002
    copyright            : (C) 2002 by the KDevelop team
    email                : team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_


#include <qmap.h>
#include <qptrlist.h>
#include <qdict.h>


#include <qextmdimainfrm.h>
#include <qextmdichildview.h>


#include "kdevmainwindow.h"

class QDomElement;
class ViewMenuAction;
class ViewMenuActionPrivateData;
class MainWindowShare;

/**\brief Handles the multiple document interface modes..


*/
class MainWindow : public QextMdiMainFrm, public KDevMainWindow
{
  Q_OBJECT

public:

  MainWindow(QWidget* parent=0, const char *name=0);
  ~MainWindow();
    
  void embedPartView(QWidget *view, const QString &title, const QString &toolTip = QString::null );    //!< Embeds a view as part view in the main working area at position (0,0)
  void embedSelectView(QWidget *view, const QString &title, const QString &toolTip);  //!< Embeds a view as select view in the left docking area
  void embedOutputView(QWidget *view, const QString &title, const QString &toolTip);  //!< Embeds a view as output view in the bottom docking area
  
  void embedSelectViewRight ( QWidget* view, const QString& title, const QString &toolTip);  //!< We do not have a right pane so this is just a wrapper to embedSelectView(

  void removeView(QWidget *view);
  void setViewAvailable(QWidget *pView, bool bEnabled);
  
  void raiseView(QWidget *view);
  void lowerView(QWidget *view);
  void lowerAllViews();

  void loadSettings();

  KMainWindow *main();                                        //!< Returns a pointer to the KMainWindow base class of this

  void init();

  void prepareToCloseViews();
  void realClose();

  virtual void setUserInterfaceMode(const QString& uiMode);
  
  virtual void callCommand(const QString& command);
  
  /** Reimplemented from QextMdiChildView to handle save prompt */
  virtual void childWindowCloseRequest(QextMdiChildView *pWnd);

  enum EView {OutputView, TreeView};

  /** Store the currently active view tab/window of the output view/window */
  void storeOutputViewTab();

  /** Restore the previously saved view tab/window to the output view/window */
  void restoreOutputViewTab();

signals:

  void wantsToQuit();

public slots:

  void createGUI(KParts::Part *part);
  void gotoNextWindow();
  void gotoPreviousWindow();
  void gotoFirstWindow();
  void gotoLastWindow();

  /** Reimplemented from QextMdiMainFrm*/
  virtual void fillWindowMenu();
  /** Reimplemented from QextMdiMainFrm to save chosen mode */
  virtual void switchToToplevelMode();
  /** Reimplemented from QextMdiMainFrm to save chosen mode */
  virtual void switchToChildframeMode();
  /** Reimplemented from QextMdiMainFrm to save chosen mode */
  virtual void switchToTabPageMode();


  /** Fills the show-hide menu for the output views */
  virtual void fillOutputToolViewsMenu();
  /** Fills the show-hide menu for the tree views */
  virtual void fillTreeToolViewsMenu();

  /** Changes the show-hide state of a single tree or output tool window */
  virtual void toggleSingleToolWin(const ViewMenuActionPrivateData &ActionData);

  /** Changes the show-hide state of a tool dock base (either output or tree tool view)*/
  virtual void toggleToolDockBaseState(const ViewMenuActionPrivateData &ActionData);

  /** Shows all tools views of a type (OutputView or TreeView*/
  virtual void showAllToolWin(EView eView,bool show);

  /** Updates the toggle state of the actions to show or hide the tool windows */
  virtual void updateActionState();

protected:
  /** */
  KParts::ReadOnlyPart * getPartFromWidget(const QWidget * pWidget) const;
  /** Reimplemented from QWidget just to get the info, that the window will now be shown */
  void resizeEvent(QResizeEvent *ev);
  /** Adds a tool view window to the output or tree views*/
  void addToolViewWindow(EView eView, QextMdiChildView *child, const QString &name, const QString &toolTip);


private slots:
  void slotQuit();
  void slotReactToProjectOpened();
  void slotRestoreAdditionalViewProperties(const QString& viewName, const QDomElement* viewEl);
  void slotSaveAdditionalViewProperties(const QString& viewName, QDomElement* viewEl);
  void slotToggleViewbar();
  
private:
  
  /** Fills the show-hide menu for a tool view (output or tree view) */
  void fillToolViewsMenu(EView eView);

  virtual bool queryClose();


/**\brief Creates a wrapper of type QextMdiChildView for the given view.

After a call to wrapper(), the given view is included in the frame of gideon in the following way:\n
- The view gets a new parent of type QextMdiChildView.
- A horizontal layout manager (QHBoxLayout) is generated which has a the new QextMdiChildView as parent and the
  given view as Widget to layout.
- The given view and the new QextMdiChildView are appended to
  \ref MainWindow::m_childViewMap "m_childViewMap" and \ref MainWindow::m_widgetMap "m_widgetMap"
- The new QextMdiChildView gets the given name as caption (full path name) and just the extracted file name as TabCaption.
.
The newly created QextMdiChildView is not yet connected to any other widget of Gideon, althoug it is appended to
\ref MainWindow::m_childViewMap "m_childViewMap" and \ref MainWindow::m_widgetMap "m_widgetMap".
*/

  QextMdiChildView *wrapper(
              QWidget *view,
              const QString &name);

  void createStatusBar();
  void createFramework();
  void createActions();

  void saveSettings();
  void saveMDISettings();
  void loadMDISettings();
  
  QMap<QWidget*,QextMdiChildView*> m_widgetMap;    //!< Key: QWidget* --> Data:QextMdiChildView*.\n
                                                   //!< Contains the same pairs as \ref MainWindow::m_childViewMap "m_childViewMap"
  QMap<QextMdiChildView*,QWidget*> m_childViewMap; //!< Key: QextMdiChildView* --> Data:QWidget*.\n
                                                   //!< Contains the same pairs as \ref MainWindow::m_widgetMap "m_widgetMap"
  QDict<QextMdiChildView>          m_captionDict;  //!< Key: caption string --> Data:QextMdiChildView*

  QPtrList<QextMdiChildView> m_outputViews;        //!< Lists all output views
  QPtrList<QextMdiChildView> m_selectViews;        //!< Lists all select views
  QPtrList<QextMdiChildView> m_partViews;          //!< Lists all part views
  QPtrList<QextMdiChildView> m_unavailableViews;   //!< Lists all views set to unavailable

  bool m_closing;                                  //!< true if we are about to close or just closing

  // Members for dealing with the tool views
  bool             m_myWindowsReady;               //!< true: gideon s windows are ready
  ViewMenuAction * m_pShowOutputViews;             //!< Shows or hides all output views
  ViewMenuAction * m_pShowTreeViews;               //!< Shows or hides all tree views
  KActionMenu *    m_pTreeToolViewsMenu;           //!< Menu for changing the show-hide state of the tree tool views
  KActionMenu *    m_pOutputToolViewsMenu;         //!< Menu for changing the show-hide state of the output tool views
  QString          m_CurrentOutputTab;             //!< Holds the previously active output tool view, if all output views have been hidden
  QString          m_CurrentTreeTab;               //!< Holds the previously active tree tool view, if all output views have been hidden
  KToggleAction*   m_toggleViewbar;

  MainWindowShare*   m_pMainWindowShare;
};

//=========================
/**\brief Data for ViewMenuAction
*/
struct ViewMenuActionPrivateData
{
  KDockWidget * pDockWidget;                        //!< Pointer to the KDockWidget to hide or show
  QextMdiChildView *pChildView;
  MainWindow::EView eView;                         //!< Which tool view the action is for
};

/**\brief Mereley a KToggleAction whith some additional data to store the window
*/
class ViewMenuAction : public KToggleAction
{
 Q_OBJECT
 public:
  ViewMenuAction(ViewMenuActionPrivateData Data, const QString &Name);
    /**
     * Constructs an action with text, icon, potential keyboard
     * shortcut, and a SLOT to call when this action is invoked by
     * the user.
     *
     * If you do not want or have a keyboard shortcut, set the
     * @p cut param to 0.
     *
     * This is the other common KAction used.  Use it when you
     * @p do have a corresponding icon.
     *
     * @param Data The data to be used when activated.
     * @param text The text that will be displayed.
     * @param pix The icon to display.
     * @param cut The corresponding keyboard shortcut.
     * @param receiver The SLOT's parent.
     * @param slot The SLOT to invoke to execute this action.
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    ViewMenuAction(
             ViewMenuActionPrivateData Data,
             const QString& text, const QString& pix, const KShortcut& cut,
             const QObject* receiver, const char* slot,
             KActionCollection* parent, const char* name );

  const ViewMenuActionPrivateData* getData(void)
    {return &WindowData;}

  signals:
    void activated (const ViewMenuActionPrivateData &);
  protected slots:
    virtual void slotActivated();

  private:
  /**\brief The data of KToggleAction*/
  ViewMenuActionPrivateData WindowData;    //!< My data to identify the window to hide or show

};


#endif
