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

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListView;
class QListViewItem;
class QMultiLineEdit;
class QPushButton;
class QPushButton;
class QButton;

class KEdit;
class KIconDialog;
class KIconLoader;
class KProcess;
class KShellProcess;
class KSeparator;

class CProject;

#include <kwizard.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qstrlist.h>


/** the KDE-Application-Wizard
  *@author Stefan Heidrich
  */

class CKAppWizard : public KWizard {
  Q_OBJECT

protected:
  void removeSources(const QString &);

public:
  CKAppWizard(QWidget* parent=0,const char* name=0,QString author_name="",QString author_email="");
  ~CKAppWizard();
  void init();
  void initPages(); 
  QString getProjectFile();
  bool generatedProject();
  void okPermited(); 
  void generateEntries(const QString &);
  QString getAuthorEmail(){return m_author_email;}
  QString getAuthorName(){return m_author_name;}
  
public slots:

void slotDirDialogClicked();
  void slotAppEnd();
  void slotPerlOut(KProcess*,char*,int);
  void slotPerlErr(KProcess*,char*,int);
  void slotHeaderDialogClicked();
  void slotCppDialogClicked();
  void slotNewHeaderButtonClicked();
  void slotNewCppButtonClicked();
  void slotDefaultClicked();
  //  void slotAppClicked();
  //  void slotMiniClicked();
  //  void slotQtClicked();
  //  void slotCPPClicked();
  void slotProjectnameEntry(const QString&);
  void slotDirectoryEntry(const QString&);
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
  void slotVendorEntry(const QString&);

// Override the QDialog accept so the dialog won't close until
// we want it too.
//  void slotOkClicked();
  void accept();

private:
  QString kdedir_env;
  QString m_author_email;
  QString  m_author_name;
  bool  gen_prj, modifyDirectory,modifyVendor;
  QObject* o;
  QWidget *widget1a,*widget1b;
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
  QWidget *page0,*page1,*page2,*page3,*page4,*page5;
  KSeparator *separator0,*separator1,*separator2;
  KIconDialog *iload,*mload;
  QPixmap pm,iconpm,minipm,*icontemp,*minitemp;
  QPainter painter;
  QString m_dir,nametext,directorytext,name1,name2,namelow,nameold;
  QString entriesfname;
  KEdit *cppedit,*hedit;
  QButton *m_finishButton, *m_cancelButton,*m_defaultButton, *m_prevButton;
  KShellProcess *q;
  //  KSimpleConfig* settings;
  CProject* project;
  QStrList sub_dir_list;
  QListViewItem *kdeentry;
  QListViewItem *kde2normalitem;
  QListViewItem *kde2miniitem;
  QListViewItem *kde2mdiitem;
  QListViewItem *qtentry;
  QListViewItem *ccppentry;
  QListViewItem *gnomeentry;
  QListViewItem *othersentry;
  QListViewItem *citem;
  QListViewItem *cppitem;
  QListViewItem *customprojitem;
  QListViewItem *gnomenormalitem;
  QListViewItem *qt2normalitem;
  QListViewItem *qt2mdiitem;
  QListViewItem *qextmdiitem;
  QListViewItem *kickeritem;
  QListViewItem *kpartitem;
  QListViewItem *sharedlibitem;
  QListViewItem *kioslaveitem;
  QListViewItem *kthemeitem;
  QListViewItem *kcmoduleitem;
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
  QString noneStr;
};

#endif
