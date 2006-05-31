/***************************************************************************
 *   Copyright (C) 2005 by Jens Herden                                     *
 *   jens@kdewebdev.org                                                    *
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

#ifndef KDEVPROJECTVIEW_H
#define KDEVPROJECTVIEW_H

#include "fileinfo.h" 

#include <qvaluelist.h>
#include <qmap.h>
#include <qguardedptr.h>

#include <kdevplugin.h>
#include <kurl.h>


class QPopupMenu;
class KAction;
class KSelectAction;
class KDialogBase;
class Context;
class ConfigWidgetProxy;
class FileListWidget;
class KToolBar;
class QVBox;
class ToolbarGUIBuilder;



/**
Please read the README.dox file for more info about this part

this is a reimplementation of the project views in Quanta

@TODO find a way to control the toolbar again
@TODO save html parts too
FIXME at project open I get the default view and the open files at last close
*/
class ProjectviewPart: public KDevPlugin
{
  Q_OBJECT
public:
 
  typedef QMap<QString, FileInfoList> ViewMap;
//   typedef QMap<KURL, FileInfo> FileInfoMap;
  
  
  ProjectviewPart(QObject *parent, const char *name, const QStringList &args);
  ~ProjectviewPart();
  
  // reimplemented from KDevPlugin
  void restorePartialProjectSession(const QDomElement * el);
  void savePartialProjectSession(QDomElement * el);
  
  /**
  * 
  * @return the current defaut view
  */
  inline QString getDefaultView() const {return m_defaultProjectView;};
  
  /**
  * 
  * @param view new default view
  */
  inline void setDefaultView(const QString& view) {m_defaultProjectView = view;};
  
  /**
  * get all current views
  * @return QStringList of defined views
  */
  inline QStringList getViewList() const {return m_projectViews.keys();};

private slots:
  void init();
  
  void insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo);
  void contextMenu(QPopupMenu *popup, const Context *context);
  void projectOpened();
  void projectClosed();
  
  /** Deletes a project view
    *
    * @param view name of the project view
    */
  void slotDeleteProjectView(const QString &view);
  
  /**
    * Deletes the current project view
    */
  void slotDeleteProjectViewCurent();

  /**
    * Saves a project view
    *
    * @param askForName true = show a dialog for the name
    */
  void slotSaveAsProjectView(bool askForName = true);
  
  /** Saves the current project view
    */
  void slotSaveProjectView();
  
  /** Opens a project view
    *
    * @param view name of the project view
    */
  void slotOpenProjectView(const QString &view);
  
private:
  /**
    * create and initialize the actions
    */
  void setupActions();
  
  /**
  * Change the actions according to the current available views
  */
  void adjustViewActions();
  
  /**
   * write the views to the plugin config file
   */
  void writeConfig();

  /**
   * read the views from the plugin config file
   */
  void readConfig();

  ViewMap m_projectViews;  //container for the views
  
  KAction *m_savePrjViewAction;
  KAction *m_newPrjViewAction;
  KAction *m_deleteCurrentPrjViewAction;
  KSelectAction *m_openPrjViewAction;
  KSelectAction *m_deletePrjViewAction;
  
  QString m_currentProjectView;
  QString m_defaultProjectView;  // load this after project loaded
  
  KURL m_projectBase;  // project base folder
  ConfigWidgetProxy *m_configProxy;

  QGuardedPtr<QWidget> m_widget;
  QGuardedPtr<ToolbarGUIBuilder> m_guibuilder;
  QWidget * m_toolbarWidget;
  
  bool m_restored;
};


#endif
