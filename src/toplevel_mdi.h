#ifndef _TOPLEVEL_MDI_H_
#define _TOPLEVEL_MDI_H_


#include <qmap.h>
#include <qptrlist.h>


#include <qextmdimainfrm.h>
#include <qextmdichildview.h>


#include "kdevtoplevel.h"



/**\brief Handles the multiple document interface modes..


*/
class TopLevelMDI : public QextMdiMainFrm, public KDevTopLevel
{
  Q_OBJECT

public:

  TopLevelMDI(QWidget* parent=0, const char *name=0);
  ~TopLevelMDI();
    
  void embedPartView(QWidget *view, const QString &title);    //!< Embeds a view as part view in the main working area at position (0,0)
  void embedSelectView(QWidget *view, const QString &title);  //!< Embeds a view as select view in the left docking area
  void embedOutputView(QWidget *view, const QString &title);  //!< Embeds a view as output view in the bottom docking area
  
  void embedSelectViewRight ( QWidget* view, const QString& title );  //!< We do not have a right pane so this is just a wrapper to embedSelectView(

  void removeView(QWidget *view);

  void raiseView(QWidget *view);
  void lowerView(QWidget *view);
  void lowerAllViews();

  void loadSettings();

  KMainWindow *main();                                        //!< Returns a pointer to the KMainWindow base class of this

  void init();

  void realClose();

  /** Reimplemented from QextMdiChildView to handle save prompt */
  virtual void childWindowCloseRequest(QextMdiChildView *pWnd);

protected:
KParts::ReadOnlyPart * getPartFromWidget(const QWidget * pWidget) const;
signals:

  void wantsToQuit();


public slots:

  void createGUI(KParts::Part *part);
  void gotoNextWindow();
  void gotoPreviousWindow();

  /** Reimplemented from QextMdiMainFrm*/
  virtual void fillWindowMenu();
  /** Reimplemented from QextMdiMainFrm to save chosen mode */
  virtual void switchToToplevelMode();
  /** Reimplemented from QextMdiMainFrm to save chosen mode */
  virtual void switchToChildframeMode();
  /** Reimplemented from QextMdiMainFrm to save chosen mode */
  virtual void switchToTabPageMode();


protected:

  void resizeEvent(QResizeEvent *ev);


private slots:

  void slotQuit();
  void slotKeyBindings();
  void slotSettings();
  void slotActiveProcessCountChanged( uint active );
  void slotShowMenuBar();
  void slotNewToolbarConfig();
  void slotConfigureToolbars();
  
private:
  
  virtual bool queryClose();
  

/**\brief Creates a wrapper of type QextMdiChildView for the given view.

After a call to wrapper(), the given view is included in the frame of gideon in the following way:\n
- The view gets a new parent of type QextMdiChildView.
- A horizontal layout manager (QHBoxLayout) is generated which has a the new QextMdiChildView as parent and the
  given view as Widget to layout.
- The given view and the new QextMdiChildView are appended to
  \ref TopLevelMDI::m_childViewMap "m_childViewMap" and \ref TopLevelMDI::m_widgetMap "m_widgetMap"
- The new QextMdiChildView gets the given name as caption (full path name) and just the extracted file name as TabCaption.
.
The newly created QextMdiChildView is not yet connected to any other widget of Gideon, althoug it is appended to
\ref TopLevelMDI::m_childViewMap "m_childViewMap" and \ref TopLevelMDI::m_widgetMap "m_widgetMap".
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
  
  KAction* m_stopProcesses;
  
  QMap<QWidget*,QextMdiChildView*> m_widgetMap;    //!< Key: QWidget* --> Data:QextMdiChildView*.\n
                                                   //!< Contains the same pairs as \ref TopLevelMDI::m_childViewMap "m_childViewMap"
  QMap<QextMdiChildView*,QWidget*> m_childViewMap; //!< Key: QextMdiChildView* --> Data:QWidget*.\n
                                                   //!< Contains the same pairs as \ref TopLevelMDI::m_widgetMap "m_widgetMap"

  QPtrList<QextMdiChildView> m_outputViews;        //!< Lists all output views
  QPtrList<QextMdiChildView> m_selectViews;        //!< Lists all select views
  QPtrList<QextMdiChildView> m_partViews;          //!< Lists all part views

  bool m_closing;                                  //!< true if we are about to close or just closing

};


#endif
