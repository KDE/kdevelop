/***************************************************************************
                          ckdevinstall.h  -  description                              
                             -------------------                                         
    begin                : Thu Mar 4 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de
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

class CKDevInstallState
{
public:
	CKDevInstallState(const QString& qtDocPath, const QString& kdeDocPath);
	int highlightStyle;	// 0 = Emacs, 1 = KWrite

  bool qt_test;
  bool kde_test;
  bool successful;

  bool glimpse;
  bool glimpseindex;
  bool htdig;
  bool htsearch;
  bool finished_glimpse;

  bool make;
  bool gmake;

  QString qt;
  QString kde;

	bool install;
	QDir* finish_dir;
	
	QString searchEngine;
	int userInterfaceMode;
	
  KShellProcess* shell_process;
};

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

class WizardBasePage : public QHBox
{
	Q_OBJECT
public:
	WizardBasePage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState = 0L);

  virtual QString pageTitle() const { return m_pageTitle; };
  virtual void setPageTitle( const QString& pageTitle ) { m_pageTitle = pageTitle; };
  virtual bool valid() const { return m_valid; }
  virtual void setValid( bool state );
  virtual void setHelpTextBackgroundColor( const QColor& bgcolor);

signals:
  void validData(QWidget* page, bool valid);

protected:
  QString m_pageTitle;
  bool    m_valid;
  QLabel * labelText;
	QVBox * m_vbox;
	CKDevInstallState* m_pInstallState;
};

class WizardWelcomePage : public WizardBasePage
{
public:
	WizardWelcomePage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename);
};

class WizardToolsCheckPage : public WizardBasePage
{
	Q_OBJECT
public:
	WizardToolsCheckPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);
	void searchForAllTools(QListBox* lb);
};

class WizardSyntaxHlPage : public WizardBasePage
{
	Q_OBJECT
public:
	WizardSyntaxHlPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);
protected slots:	
	void slotSetSyntaxHl(int type);
};

class WizardQtDocPage : public WizardBasePage
{
	Q_OBJECT
public:
	WizardQtDocPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);
protected:
  virtual void showEvent( QShowEvent * e);
	virtual void hideEvent( QHideEvent * e);
private slots:	
	void slotQTpressed();
private:
	QLineEdit* qt_edit;
};

class WizardKDEDocPage : public WizardBasePage
{
	Q_OBJECT
public:
	WizardKDEDocPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);
protected:
  virtual void showEvent( QShowEvent * e);
	virtual void hideEvent( QHideEvent * e);
signals:	
	void enableCreateKDEDocPage(bool);
private slots:	
	void slotKDEpressed();
};

class WizardCreateKDEDocPage : public WizardBasePage
{
	Q_OBJECT
public:
	WizardCreateKDEDocPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);
protected slots:	
	void slotSetKDEDocPath(const QString& newKDEDocPath);
};

class WizardIndexDocuPage : public WizardBasePage
{
	Q_OBJECT
public:
	WizardIndexDocuPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);
protected slots:	
	void indexingFinished(const QString&);
};

class WizardUIModePage : public WizardBasePage
{
	Q_OBJECT
public:
	WizardUIModePage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);
protected slots:	
	void slotModeChanged(int userInterfaceMode);
};

class WizardLastPage : public WizardBasePage
{
public:
	WizardLastPage(QWidget* parent, const char* name, const QString& infoText, const QString& installPictPathAndFilename, CKDevInstallState* pInstallState);
};

#endif
