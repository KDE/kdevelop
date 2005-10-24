/***************************************************************************
  newmainwindow.h
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
#ifndef _NEWMAINWINDOW_H_
#define _NEWMAINWINDOW_H_


#include <kmdimainfrm.h>
#include <kparts/mainwindow.h>

#include <qdatetime.h>
#include <qmap.h>
#include <qguardedptr.h>

#include <kdevpartcontroller.h>
#include "kdevmainwindow.h"

class MainWindowShare;

/**
Main window implementation.
*/
class NewMainWindow : public KMdiMainFrm, public KDevMainWindow
{
  Q_OBJECT

public:

  NewMainWindow(QWidget* parent=0, const char *name=0, KMdi::MdiMode = KMdi::IDEAlMode );
  ~NewMainWindow();

  void embedPartView(QWidget *view, const QString &title, const QString &toolTip);
  void embedSelectView(QWidget *view, const QString &title, const QString &toolTip);
  void embedSelectViewRight ( QWidget* view, const QString& title, const QString &toolTip);
  void embedOutputView(QWidget *view, const QString &title, const QString &toolTip);
  void embedView( KDockWidget::DockPosition pos, QWidget *view, const QString &name, const QString &toolTip);

  void removeView(QWidget *view);
  void setViewAvailable(QWidget *pView, bool bEnabled);

  void raiseView(QWidget *view);
  void lowerView(QWidget *view);

  void loadSettings();
  void showTabs( bool showTabs );

  /** Reimplemented from KMdiChildView to handle save prompt */
  virtual void childWindowCloseRequest( KMdiChildView * );
  
  KMainWindow *main();

  void init();

  //This is needed because KMdiMainFrame::tabWidgetVisibility() is protected (for
  //whatever reason) and this value is needed in EditorProxy
  KMdi::TabWidgetVisibility getTabWidgetVisibility() { return tabWidgetVisibility(); }
  
  /**Enable the main window to be shown. Main window is created with m_canBeShown
  flag set to false. This sets the flag to true and show() method will be enabled.*/
  void enableShow();
    
public slots:

  void createGUI(KParts::Part *part);
  void raiseEditor();

  /** Just after the project gets opened */
  void projectOpened();
  
  /** called by the part to set the caption */
  void setCaption( const QString &);
  
  virtual void show();
  
signals:
  void currentChanged( QWidget* widget );
  void sigCloseWindow( const QWidget* widget );

private slots:
  void slotViewActivated(KMdiChildView* child);
  void slotCoreInitialized();
  void slotPartURLChanged( KParts::ReadOnlyPart *);
  void documentChangedState( const KURL &, DocumentState );
  void fillWindowMenu();
  void openURL( int );
  void tabMoved(int from, int to);
  void tabContext(QWidget*,const QPoint &);  
  void tabContextActivated(int);    
  void configureToolbars();
  void setupWindowMenu();
  void slotNewToolbarConfig();

//  void loadMDISettings();

private:

	KDockWidget::DockPosition recallToolViewPosition( const QString & widgetName, KDockWidget::DockPosition defaultPos );
	void rememberToolViewPosition( const QString & widgetName, KDockWidget::DockPosition pos );
	struct ToolViewData
	{
		ToolViewData() {}
		ToolViewData( KDockWidget::DockPosition pos, QString n, QString t ) 
			: position(pos), name(n), toolTip(t) {}

		KDockWidget::DockPosition position;
		QString name;
		QString toolTip;
	};

	QMap<QWidget*,ToolViewData> m_availableToolViews;
	QMap<QWidget*,ToolViewData> m_unAvailableToolViews;

  virtual bool queryClose();
  virtual bool queryExit();

  void createStatusBar();
  void createFramework();
  void createActions();

  void saveSettings();

  void moveRelativeTab(int);
  
  KAction *m_raiseEditor;

  QValueList< QPair< int, KURL > > m_windowList;
  MainWindowShare*   m_pMainWindowShare;
  
  bool openNewTabAfterCurrent;
  bool showTabIcons;

  KURL m_currentTabURL;

  bool m_canBeShown;
};


#endif
