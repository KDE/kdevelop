/***************************************************************************
                 ctoolclass.h - some methods,that would be normal global,but I
		              hate global function :-)
                             -------------------                                         

    begin                : 20 Jan 1999                        
    copyright            : (C) 1999 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef CTOOLCLASS_H
#define CTOOLCLASS_H

#include <stdlib.h>

#include <qstring.h>
#include <qvaluelist.h>
#include <kconfig.h>

/**
 * This is the representation of an application that is shown in the Tools Menu
 */
class CToolApp {

public:

  /** Default Constructor (never use it, please) */
  CToolApp(): m_captureOutput(false), m_inNewPane(false) {}

  /** label: What will be displayed in the Menu
      exeName: Name of executable
      args: Arguments to pass to the app
      captureOutput: capture STDOUT and STDERR from this app
      inNewPane: Open a new pane for this app (not implemented yet)
  */
  CToolApp(QString label, QString exeName, QString args = QString::null, bool captureOutput = false, bool inNewPane = false):
        m_label(label), m_exeName(exeName), m_args(args), m_captureOutput(captureOutput), m_inNewPane(inNewPane) {}


  QString getLabel() const            { return m_label; }
  QString getExeName() const          { return m_exeName; }
  QString getArgs() const             { return m_args; }
  bool    isOutputCaptured() const    { return m_captureOutput; }
  bool    isInNewPane() const         { return m_inNewPane; }

private:
  QString m_label, m_exeName, m_args;
  bool m_captureOutput, m_inNewPane;
};

typedef QValueList<CToolApp> ToolAppList;

/**
 * some methods,that would be normaly global,but I hate global functions :-)
 * @author Sandy Meier
 */
class CToolClass {
public:
  /** search the program in the $PATH*/
  static bool searchProgram(const QString& name, bool allowWarningMsg=true);
  /** same as above but without warning message*/
  static bool searchInstProgram(const QString& name) { return searchProgram( name, false); };

  /** return the absolute filename if found, otherwise ""*/
  static QString findProgram(const QString& name);
  /** returns the relative path, to get from source_path to destpath (both are absolute)
      for examples:
      source_path: /home/smeier/testprj/
      dest_path:   /usr/lib/
      relative_path = ../../../user/lib/
   */
  static QString getRelativePath(QString source_dir,QString dest_dir);

  /** reads the Tool Configuration from Config File */
  static void readToolConfig(ToolAppList& toolList);

  /** writes the Tool Configuration to Config File */
  static void writeToolConfig(const ToolAppList& toolList);
};

#endif


