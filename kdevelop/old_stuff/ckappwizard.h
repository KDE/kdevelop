/***************************************************************************
                    ckappwizard.h - the kde-application-wizard
                             -------------------                                         

    version              :                                   
    begin                : 9 Sept 1998                                        
    copyright            : (C) 1998 by Stefan Heidrich                         
    email                : sheidric@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef CKAPPWIZARD_H
#define CKAPPWIZARD_H

#include <qstrlist.h>
#include <qpixmap.h>
#include <kwizard.h>

class CProject;
class QListView;
class QListViewItem;
class QLabel;
class QPushButton;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QMultiLineEdit;
class QButtonGroup;
class KSeparator;
class KEdit;
class KShellProcess;
class KProcess;


/** the KDE-Application-Wizard
  *@author Stefan Heidrich
  */

class CKAppWizard : public KWizard {
  Q_OBJECT
public:
  CKAppWizard(QWidget* parent=0,const char* name=0,QString author_name="",QString author_email="");
  ~CKAppWizard();
  void init();
  void initPages(); 
  QString getProjectFile();
  bool generatedProject();
  void okPermited(); 
  QString getAuthorEmail(){return m_author_email;}
  QString getAuthorName(){return m_author_name;}
  
public slots:

void slotDirDialogClicked();
  void slotPerlOut(KProcess*,char*,int);
  void slotPerlErr(KProcess*,char*,int);
  void slotHeaderDialogClicked();
  void slotCppDialogClicked();
  void slotNewHeaderButtonClicked();
  void slotNewCppButtonClicked();
  void slotDefaultClicked(int);
  //  void slotAppClicked();
  //  void slotMiniClicked();
  //  void slotQtClicked();
  //  void slotCPPClicked();
  void slotProjectnameEntry();
  void slotDirectoryEntry();
  void slotIconButtonClicked();
  void slotMiniIconButtonClicked();
  void slotProgIconClicked();
  void slotMiniIconClicked();  
  void slotHeaderHeaderClicked();
  void slotCppHeaderClicked(); 
  void slotProcessExited();
  void slotMakeEnd();
  void slotApplicationClicked();
  void slotLocationButtonClicked();
  void slotVSBoxChanged(int);
  void slotVendorEntry();

protected slots:
  virtual void accept();
  virtual void reject();

private:

  void copyFile(QString source, QString dest);
  QString m_author_email;
  QString  m_author_name;
  bool  gen_prj, modifyDirectory,modifyVendor;
  QObject* o;
  QWidget *widget0,*widget1,*widget1c,*widget2,*widget3,*widget4,*widget1a,*widget1b;
  QListView* applications;
  QLabel* apphelp;
  QPushButton* directoryload;
  QLineEdit* emailline;
  QLineEdit* authorline;
  QLineEdit* versionline;
  QLineEdit* nameline;
  QLineEdit* directoryline;
  QLabel* name;
  QLabel* email;
  QLabel* authorname;
  QLabel* versionnumber;
  QLabel* directory;
  QCheckBox* datalink;
  QCheckBox* miniicon;
  QCheckBox* progicon;
  QCheckBox* gnufiles;
  QCheckBox* lsmfile;
  QCheckBox* userdoc;
  QCheckBox* apidoc;
  QPushButton* miniload;
  QPushButton* iconload;
  QCheckBox* generatesource;
  QPushButton* hload;
  QPushButton* hnew;
  QCheckBox* hheader;
  QPushButton* cppnew;
  QPushButton* cppload;
  QCheckBox* cppheader;
  QMultiLineEdit* output;
  QMultiLineEdit* errOutput;
  KSeparator *separator0,*separator1,*separator2;
  QPixmap pm,iconpm,minipm,*icontemp,*minitemp;
    //  KApplication* help;
  QString dir,nametext,directorytext,name1,name2,namelow,nameold;
  KEdit *cppedit,*hedit;
  KShellProcess *q;
  //  KSimpleConfig* settings;
  CProject *project;
  QStrList sub_dir_list;
  QListViewItem *kdeentry;
  //QListViewItem *corbaitem;
  //QListViewItem *komitem;
  QListViewItem *kdenormalitem;
  QListViewItem *kdeminiitem;
  QListViewItem *qtentry;
  QListViewItem *ccppentry;
  //QListViewItem *gtkentry;
  QListViewItem *othersentry;
  QListViewItem *citem;
  QListViewItem *cppitem;
  QListViewItem *customprojitem;
  //QListViewItem *gtknormalitem;
  // QListViewItem *gtkminiitem;
  QListViewItem *qtnormalitem;
  //  QListViewItem *qtminiitem;
  QLineEdit* messageline;
  QLabel* logMessage;
  QLineEdit* vendorline;
  QLabel* vendorTag;
  QLineEdit* releaseline;
  QLabel* releaseTag;
  QLabel* vsInstall;
  QLabel* projectVSLocation;
  QLineEdit* projectlocationline;
  QLineEdit* vsLocation;
  QComboBox* vsBox;
  QLabel* vsSupport;
  QPushButton* locationbutton;
  QButtonGroup* qtarch_ButtonGroup_1;
};

#endif



