/***************************************************************************
                          kpp.h  -  description
                             -------------------
    begin                : Sun Oct 10 16:16:40 CDT 1999
    copyright            : (C) 1999 by ian geiser
    email                : geiseri@msoe.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KPP_H
#define KPP_H

#include <kprocess.h>
#include <kapp.h>
#include <qwidget.h>
#include <kprocess.h>
#include "projectView.h"
#include "cdevproject.h"
#include "cspec.h"
class Kpp : public projectview {
  Q_OBJECT
public:
  /** construtor */
  Kpp(QWidget*parent=0,const char* name=0, bool modal=false);
  /** destructor */
  ~Kpp();
  /** Save the current enviroment information for the application. */
  bool saveClass();
  /** Load the user spacific prefs for the application. */
  bool loadPrefs();
  /** Save all of the current application preferences */
  void savePrefs();
  /** Set the current config path */
  void setConfig( KConfig *theConfig);
  /** start the rpm build process */
  bool startBuild();
  /** allow kdevelop to set up the kpp dialog **/
  void setProjectData(QString appName, QString appVer, QString appAuth, QString appEmail, QString configOpts, QString appDesc);

private: // Private methods
  /** Update the spec object to the current data in the dialog */
  void updateSpec();

  /** Generates a spec file from a template. */
  bool generateSpec(QString fileName);
  /** Runs RPM -ba */

  /** The spec file of the current project */
  cSpec currentSpec;
  /** The current progress of the RPM build */
 // buildView *bldView;
  /** The base for the rpm build directories. */
  QString qsRPMBaseDir;
  /** The RPM command */
  QString qsRPMCpmmand;
  /** the specfile template */
  QString qsSpecTemplate;
  /** Current Source Code */
  QString qsCurrentSource;
  /** The build process */
  KProcess proc;
  /** The current spec file location. */
  QString qsCurrentSpec;
  /** The current config of the application. we are going to hold EVERY thing in
	so that we can have a sort of session management. */
  KConfig *kcConfig;
  /** the rpm build process */
  KShellProcess *rpmBuild;
  /** The project from the current working directory */
  ckdevelProject *currentProject;

public slots: // Public slots

  /** This will allow the user to save the variables for the current project. */
  void saveFile();
  /** Opens a KPP project file. */
  void openFile();
  /** Opens the online help */
  void helpMe();
  /** Exit's the application. */
  void exitApp();
  /** Closes the build window */

  /** Exit the current preferences */
  void editPrefs();

  /** This is a default dialog for things not yet implemented. */
  void notYet();
  /** Change the template that we will use to generate the spec file. */
  void changeSpec();

signals:
        // done withthe dialog, so tell the main app to destroy me
        void finished();
        // building, so hide the dialog but keep the build process
        // going
        void building();

private slots: // Private slots
  /** done building... */
  void buildDone(KProcess *proc);
  /** read teh standard error */
  void readStdErr(KProcess *proc, char *buffer, int buflen);
  /** read the standard out */
  void readStdOut(KProcess *proc, char *buffer, int buflen);
  /**  */
  void rpmBuildSlot();
};
#endif






































