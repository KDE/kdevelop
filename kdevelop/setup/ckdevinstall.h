/***************************************************************************
                          ckdevinstall.h  -  description                              
                             -------------------                                         
    begin                : Thu Mar 4 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    rewrite              : (C) Jun 2001 by Falk Brettschneider
    email                : Ralf.Nolden@post.rwth-aachen.de
                         : falk.brettschneider@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef CKDEVINSTALL_H
#define CKDEVINSTALL_H

#include <qdialog.h>
#include <qhbox.h>
#include <qwizard.h>

class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QListBox;
class QVBox;
class QDir;
class KConfig;
class KProcess;
class KShellProcess;
class CKDevInstallState;

/**Provides an installation module checking
  *installed programs needed by KDevelop. Uses CToolClass
  *for checking programs.
  *@author Ralf Nolden
  */

class CKDevInstall : public QWizard
{
   Q_OBJECT
public:
	CKDevInstall(QWidget *parent=0, const char *name=0);
	~CKDevInstall();

protected slots:
  void slotHelp();
  void slotCancel();
  void slotReceivedStdout(KProcess*,char*,int);
  void slotReceivedStderr(KProcess*,char*,int);
  void slotProcessExited(KProcess*);
	void slotFinished();
	void slotEnableCreateKDEDocPage(bool bEnabled);

protected:
	KConfig *m_config;
	CKDevInstallState* m_pInstallState;
};

#endif
