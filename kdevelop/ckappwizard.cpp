/***************************************************************************

                    kappwizard.cpp - the kde-application-wizard
                             -------------------

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

#include "ckappwizard.h"

#include "ceditwidget.h"
#include <cproject.h>
#include "ctoolclass.h"
#include "debug.h"

#include <kconfig.h>
#include <keditcl.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kseparator.h>
#include <kstddirs.h>

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qfile.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <knumvalidator.h>

#include <qlayout.h>
#include <qgrid.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


CKAppWizard::CKAppWizard(QWidget* parent,const char* name,QString author_name,QString author_email) :
  KWizard(parent,name,true)
{
  q = new KShellProcess("/bin/sh");
  gen_prj = false;
  modifyDirectory = false;
  modifyVendor = false;
  nameold = "";

  setCaption(i18n("Application Wizard"));
  setMinimumSize(515,530);

  m_cancelButton = cancelButton();
  QToolTip::add(m_cancelButton,i18n("exit the CKAppWizard"));
  connect(m_cancelButton,SIGNAL(clicked()),this,SLOT(slotAppEnd()));

  m_finishButton = finishButton();
  m_finishButton->setText(i18n("Create"));
  QToolTip::add( m_finishButton,i18n("creating the project"));

  m_defaultButton = helpButton();
  m_defaultButton->setText(i18n("Default"));
  QToolTip::add(m_defaultButton, i18n("set all changes back"));
  connect(m_defaultButton,SIGNAL(clicked()),this,SLOT(slotDefaultClicked()));

  initPages();
  project=0l;
  m_author_email = author_email;
  m_author_name = author_name;
  slotDefaultClicked();
}

CKAppWizard::~CKAppWizard ()
{
  delete q;
  delete project;
}

void CKAppWizard::initPages()
{
  // create the first page
  page0 = new QWidget(this);
  page0->setEnabled(true);
  addPage(page0, i18n("Applications"));
  setFinishEnabled ( page0, false );

  QPixmap iconpm;
  QPixmap minipm;
  QPixmap pm;
  pm.load(locate("appdata", "pics/kAppWizard.png"));
  QPixmap pix = SmallIcon("fileopen");

  // create a widget and paint a picture on it
  widget1a = new QWidget( page0, "widget1a" );
  widget1a->setFixedSize(pm.width(), pm.height());
  widget1a->setFocusPolicy( QWidget::NoFocus );
  widget1a->setBackgroundMode( QWidget::PaletteBackground );
  widget1a->setBackgroundPixmap (pm);
  QVBoxLayout* vbl1 = new QVBoxLayout(page0, 8, 4);
  QHBoxLayout* hbl1 = new QHBoxLayout(vbl1, 10);
  hbl1->addWidget(widget1a);
  hbl1->addStretch();

  applications = new QListView( page0, "applications" );
  applications->setMinimumSize( 200, 250 );
  applications->setFocusPolicy( QWidget::TabFocus );
  applications->setBackgroundMode( QWidget::PaletteBackground );
  QHBoxLayout* hbl2 = new QHBoxLayout(vbl1, 10);
  hbl2->addWidget(applications);

  applications->setResizePolicy( QScrollView::Manual );
  applications->setVScrollBarMode( QScrollView::Auto );
  applications->setHScrollBarMode( QScrollView::Auto );
  applications->setTreeStepSize( 20 );
  applications->setMultiSelection( FALSE );
  applications->setAllColumnsShowFocus( FALSE );
  applications->setItemMargin( 1 );
  applications->setRootIsDecorated( TRUE );
  applications->addColumn( i18n("Applications"), -1 );
  applications->setColumnWidthMode( 0, QListView::Maximum );
  applications->setColumnAlignment( 0, 1 );
  applications->setSorting (-1,FALSE);
  applications->header()->hide();

  // create another widget for a picture
  widget1b = new QWidget( page0, "widget1b" );
  widget1b->setFixedSize(190, 140);
  widget1b->setFocusPolicy( QWidget::NoFocus );
  widget1b->setBackgroundMode( QWidget::PaletteBackground );
  QVBoxLayout* vbl2 = new QVBoxLayout(hbl2, 10);
  QHBoxLayout* hbl3 = new QHBoxLayout(vbl2, 10);
  hbl3->addWidget(widget1b);
  hbl3->addStretch();

  apphelp = new QLabel( page0, "apphelp" );
  apphelp->setFocusPolicy( QWidget::NoFocus );
  apphelp->setBackgroundMode( QWidget::PaletteBackground );
  apphelp->setText( i18n("Label:") );
  apphelp->setAlignment( Qt::WordBreak);//1313 );
  apphelp->setMargin( -1 );
  vbl2->addWidget(apphelp);

  othersentry = new QListViewItem (applications, i18n("Others"));
  othersentry->setExpandable (true);
  othersentry->setOpen (TRUE);
  othersentry->sortChildItems (0,FALSE);
  customprojitem = new QListViewItem (othersentry,i18n("custom project"));

  /*  gtkentry = new QListViewItem (applications, "GTK");
  gtkentry->setExpandable (true);
  gtkentry->setOpen (TRUE);
  gtkentry->sortChildItems (0,FALSE);
  gtkminiitem = new QListViewItem (gtkentry,"Mini");
  gtknormalitem = new QListViewItem (gtkentry,"Normal");
  */
  ccppentry = new QListViewItem (applications, i18n("Terminal"));
  ccppentry->setExpandable (true);
  ccppentry->setOpen (TRUE);
  ccppentry->sortChildItems (0,FALSE);
  cppitem = new QListViewItem (ccppentry,i18n("C++"));
  citem = new QListViewItem (ccppentry,i18n("C"));
  //sharedlibitem = new QListViewItem(ccppentry, i18n("C++ Shared Library"));

  qtentry = new QListViewItem (applications, i18n("Qt"));
  qtentry->setExpandable (true);
  qtentry->setOpen (TRUE);
  qtentry->sortChildItems (0,FALSE);
  qextmdiitem = new QListViewItem( qtentry, i18n("QextMDI"));
  qt2mdiitem = new QListViewItem( qtentry, i18n("Qt MDI"));
  qt2normalitem = new QListViewItem( qtentry, i18n("Qt SDI"));

  gnomeentry = new QListViewItem (applications, "GNOME");
  gnomeentry->setExpandable (true);
  gnomeentry->setOpen (TRUE);
  gnomeentry->sortChildItems (0,FALSE);
  gnomenormalitem = new QListViewItem (gnomeentry,i18n("Normal"));


  kdeentry = new QListViewItem (applications,i18n("KDE"));
  kdeentry->setExpandable (true);
  kdeentry->setOpen (TRUE);
  kdeentry->sortChildItems (0,FALSE);
  kthemeitem = new QListViewItem(kdeentry, i18n("KDE Desktop Theme"));
  kioslaveitem = new QListViewItem(kdeentry, i18n("KDE Kio Slave"));
  kickeritem = new QListViewItem( kdeentry, i18n("KDE Kicker Applet"));
  kpartitem = new QListViewItem( kdeentry, i18n("Konqueror Plugin"));
  kcmoduleitem = new QListViewItem(kdeentry, i18n("KDE KControl Module"));
  kde2mdiitem = new QListViewItem (kdeentry,i18n("KDE MDI"));
  kde2normalitem = new QListViewItem (kdeentry,i18n("KDE Normal"));
  kde2miniitem = new QListViewItem (kdeentry,i18n("KDE Mini"));
  applications->setFrameStyle( QListView::Panel | QListView::Sunken );
  applications->setLineWidth( 2 );

  connect (applications,SIGNAL(selectionChanged ()),SLOT(slotApplicationClicked()));

  /************************************************************/

  // create the second page
  page1 = new QWidget(this);
  page1->setEnabled(true);
  addPage(page1, i18n("Generate settings"));
  setFinishEnabled ( page1, true );

  name = new QLabel( page1, "name" );
  name->setGeometry( 30, 10, 100, 30 );
  name->setMinimumSize( 0, 0 );
  name->setMaximumSize( 32767, 32767 );
  name->setFocusPolicy( QWidget::NoFocus );
  name->setBackgroundMode( QWidget::PaletteBackground );
  name->setText( i18n("Project name:") );
  name->setAlignment( 289 );
  name->setMargin( -1 );

  nameline = new QLineEdit( page1, "nameline" );
  nameline->setGeometry( 140, 10, 290, 30 );
  nameline->setMinimumSize( 0, 0 );
  nameline->setMaximumSize( 32767, 32767 );
  nameline->setFocusPolicy( QWidget::StrongFocus );
  nameline->setBackgroundMode( QWidget::PaletteBase );
  nameline->setText( "" );
  nameline->setMaxLength( 32767 );
  nameline->setEchoMode( QLineEdit::Normal );
  nameline->setFrame( TRUE );

  directory = new QLabel( page1, "directory" );
  directory->setGeometry( 30, 50, 100, 30 );
  directory->setMinimumSize( 0, 0 );
  directory->setMaximumSize( 32767, 32767 );
  directory->setFocusPolicy( QWidget::NoFocus );
  directory->setBackgroundMode( QWidget::PaletteBackground );
  directory->setText( i18n("Project directory:") );
  directory->setAlignment( 289 );
  directory->setMargin( -1 );

  directoryline = new QLineEdit( page1, "directoryline" );
  directoryline->setGeometry( 140, 50, 290, 30 );
  directoryline->setMinimumSize( 0, 0 );
  directoryline->setMaximumSize( 32767, 32767 );
  directoryline->setFocusPolicy( QWidget::StrongFocus );
  directoryline->setBackgroundMode( QWidget::PaletteBase );
  directoryline->setText( "" );
  directoryline->setMaxLength( 32767 );
  directoryline->setEchoMode( QLineEdit::Normal );
  directoryline->setFrame( TRUE );

  directoryload = new QPushButton( page1, "directoryload" );
  directoryload->setGeometry( 440, 50, 30, 30 );
  directoryload->setMinimumSize( 0, 0 );
  directoryload->setMaximumSize( 32767, 32767 );
  directoryload->setFocusPolicy( QWidget::TabFocus );
  directoryload->setBackgroundMode( QWidget::PaletteBackground );
  directoryload->setPixmap(pix);
  directoryload->setAutoRepeat( FALSE );
  directoryload->setAutoResize( FALSE );

  versionnumber = new QLabel( page1, "versionnumber" );
  versionnumber->setGeometry( 30, 90, 100, 30 );
  versionnumber->setMinimumSize( 0, 0 );
  versionnumber->setMaximumSize( 32767, 32767 );
  versionnumber->setFocusPolicy( QWidget::NoFocus );
  versionnumber->setBackgroundMode( QWidget::PaletteBackground );
  versionnumber->setText( i18n("Version number:") );
  versionnumber->setAlignment( 289 );
  versionnumber->setMargin( -1 );

  versionline = new QLineEdit( page1, "versionline" );
  versionline->setGeometry( 140, 90, 290, 30 );
  versionline->setMinimumSize( 0, 0 );
  versionline->setMaximumSize( 32767, 32767 );
  versionline->setFocusPolicy( QWidget::StrongFocus );
  versionline->setBackgroundMode( QWidget::PaletteBase );
  versionline->setText( "" );
//  versionline->setValidator( new KFloatValidator( versionline ) );
  versionline->setMaxLength( 32767 );
  versionline->setEchoMode( QLineEdit::Normal );
  versionline->setFrame( TRUE );

  authorname = new QLabel( page1, "authorname" );
  authorname->setGeometry( 30, 130, 100, 30 );
  authorname->setMinimumSize( 0, 0 );
  authorname->setMaximumSize( 32767, 32767 );
  authorname->setFocusPolicy( QWidget::NoFocus );
  authorname->setBackgroundMode( QWidget::PaletteBackground );
  authorname->setText( i18n("Author:") );
  authorname->setAlignment( 289 );
  authorname->setMargin( -1 );

  authorline = new QLineEdit( page1, "authorline" );
  authorline->setGeometry( 140, 130, 290, 30 );
  authorline->setMinimumSize( 0, 0 );
  authorline->setMaximumSize( 32767, 32767 );
  authorline->setFocusPolicy( QWidget::StrongFocus );
  authorline->setBackgroundMode( QWidget::PaletteBase );
  authorline->setText( "" );
  authorline->setMaxLength( 32767 );
  authorline->setEchoMode( QLineEdit::Normal );
  authorline->setFrame( TRUE );

  email = new QLabel( page1, "email" );
  email->setGeometry( 30, 170, 100, 30 );
  email->setMinimumSize( 0, 0 );
  email->setMaximumSize( 32767, 32767 );
  email->setFocusPolicy( QWidget::NoFocus );
  email->setBackgroundMode( QWidget::PaletteBackground );
  email->setText( i18n("Email:") );
  email->setAlignment( 289 );
  email->setMargin( -1 );

  emailline = new QLineEdit( page1, "emailline" );
  emailline->setGeometry( 140, 170, 290, 30 );
  emailline->setMinimumSize( 0, 0 );
  emailline->setMaximumSize( 32767, 32767 );
  emailline->setFocusPolicy( QWidget::StrongFocus );
  emailline->setBackgroundMode( QWidget::PaletteBase );
  emailline->setText( "" );
  emailline->setMaxLength( 32767 );
  emailline->setEchoMode( QLineEdit::Normal );
  emailline->setFrame( TRUE );

  generatesource = new QCheckBox( page1, "generatesource" );
  generatesource->setGeometry( 30, 220, 440, 30 );
  generatesource->setMinimumSize( 0, 0 );
  generatesource->setMaximumSize( 32767, 32767 );
  generatesource->setFocusPolicy( QWidget::TabFocus );
  generatesource->setBackgroundMode( QWidget::PaletteBackground );
  generatesource->setText( i18n("generate sources and headers") );
  generatesource->setAutoRepeat( FALSE );
  generatesource->setAutoResize( FALSE );

  gnufiles = new QCheckBox( page1, "gnufiles" );
  gnufiles->setGeometry( 30, 270, 440, 30 );
  gnufiles->setMinimumSize( 0, 0 );
  gnufiles->setMaximumSize( 32767, 32767 );
  gnufiles->setFocusPolicy( QWidget::TabFocus );
  gnufiles->setBackgroundMode( QWidget::PaletteBackground );
  gnufiles->setText( i18n("GNU-Standard-Files (INSTALL,README,COPYING...)" ));
  gnufiles->setAutoRepeat( FALSE );
  gnufiles->setAutoResize( FALSE );

  userdoc = new QCheckBox( page1, "userdoc" );
  userdoc->setGeometry( 30, 300, 330, 30 );
  userdoc->setMinimumSize( 0, 0 );
  userdoc->setMaximumSize( 32767, 32767 );
  userdoc->setFocusPolicy( QWidget::TabFocus );
  userdoc->setBackgroundMode( QWidget::PaletteBackground );
  userdoc->setText( i18n("User-Documentation") );
  userdoc->setAutoRepeat( FALSE );
  userdoc->setAutoResize( FALSE );

  apidoc = new QCheckBox( page1, "apidoc" );
  apidoc->setGeometry( 30, 330, 200, 30 );
  apidoc->setMinimumSize( 0, 0 );
  apidoc->setMaximumSize( 32767, 32767 );
  apidoc->setFocusPolicy( QWidget::TabFocus );
  apidoc->setBackgroundMode( QWidget::PaletteBackground );
  apidoc->setText( i18n("API-Documentation") );
  apidoc->setAutoRepeat( FALSE );
  apidoc->setAutoResize( FALSE );

  lsmfile = new QCheckBox( page1, "lsmfile" );
  lsmfile->setGeometry( 30, 360, 340, 30 );
  lsmfile->setMinimumSize( 0, 0 );
  lsmfile->setMaximumSize( 32767, 32767 );
  lsmfile->setFocusPolicy( QWidget::TabFocus );
  lsmfile->setBackgroundMode( QWidget::PaletteBackground );
  lsmfile->setText( i18n("lsm-File - Linux Software Map") );
  lsmfile->setAutoRepeat( FALSE );
  lsmfile->setAutoResize( FALSE );

  datalink = new QCheckBox( page1, "datalink" );
  datalink->setGeometry( 30, 390, 200, 30 );
  datalink->setMinimumSize( 0, 0 );
  datalink->setMaximumSize( 32767, 32767 );
  datalink->setFocusPolicy( QWidget::TabFocus );
  datalink->setBackgroundMode( QWidget::PaletteBackground );
  datalink->setText( i18n(".desktop-File") );
  datalink->setAutoRepeat( FALSE );
  datalink->setAutoResize( FALSE );

  progicon = new QCheckBox( page1, "progicon" );
  progicon->setGeometry( 290, 330, 110, 30 );
  progicon->setMinimumSize( 0, 0 );
  progicon->setMaximumSize( 32767, 32767 );
  progicon->setFocusPolicy( QWidget::TabFocus );
  progicon->setBackgroundMode( QWidget::PaletteBackground );
  progicon->setText( i18n("Program-Icon") );
  progicon->setAutoRepeat( FALSE );
  progicon->setAutoResize( FALSE );

  iconload = new QPushButton( page1, "iconload" );
  iconload->setGeometry( 410, 310, 60, 60 );
  iconload->setMinimumSize( 0, 0 );
  iconload->setMaximumSize( 32767, 32767 );
  iconload->setFocusPolicy( QWidget::TabFocus );
  iconload->setBackgroundMode( QWidget::PaletteBackground );
  iconload->setText( "" );
  iconload->setAutoRepeat( FALSE );
  iconload->setAutoResize( FALSE );

  miniicon = new QCheckBox( page1, "miniicon" );
  miniicon->setGeometry( 290, 390, 110, 30 );
  miniicon->setMinimumSize( 0, 0 );
  miniicon->setMaximumSize( 32767, 32767 );
  miniicon->setFocusPolicy( QWidget::TabFocus );
  miniicon->setBackgroundMode( QWidget::PaletteBackground );
  miniicon->setText( i18n("Mini-Icon") );
  miniicon->setAutoRepeat( FALSE );
  miniicon->setAutoResize( FALSE );

  miniload = new QPushButton( page1, "miniload" );
  miniload->setGeometry( 440, 390, 30, 30 );
  miniload->setMinimumSize( 0, 0 );
  miniload->setMaximumSize( 32767, 32767 );
  miniload->setFocusPolicy( QWidget::TabFocus );
  miniload->setBackgroundMode( QWidget::PaletteBackground );
  miniload->setText( "" );
  miniload->setAutoRepeat( FALSE );
  miniload->setAutoResize( FALSE );

  separator1 = new KSeparator (page1);
  separator1->setGeometry(0,210,515,5);

  separator2 = new KSeparator (page1);
  separator2->setGeometry(0,255,515,5);

  page1->setMinimumSize(515, 430);
  
  QString nameMsg = i18n("Insert your project name here. This is\n"
               "also the name of the directory where your Project\n"
               "will be created.");
  QWhatsThis::add(name, nameMsg);
  QWhatsThis::add(nameline, nameMsg);

  QWhatsThis::add(directory, i18n("Enter the toplevel-directory of your project.\n"));
  QWhatsThis::add(directoryline,
              i18n("Enter the toplevel-directory of your project.\n"));
  QWhatsThis::add(directoryload,
              i18n("Enter the toplevel-directory of your project.\n"));
  QString versionMsg = i18n("Set the initial version number of your project here.\n"
               "The number will be used in the about-dialog as well as for\n"
               "determining the project's package numbering for distribution.");

  QWhatsThis::add(versionnumber, versionMsg);
  QWhatsThis::add(versionline, versionMsg);

  QString authorMsg = i18n("Insert your name or the development team name here. This will be used\n"
                          "for adding your name as the author to all generated files of your project.");
  QWhatsThis::add(authorname, authorMsg);
  QWhatsThis::add(authorline, authorMsg);

  QString emailMsg = i18n("Enter your email adress here. This will be\n"
               "used for file header information.");
  QWhatsThis::add(email, emailMsg);
  QWhatsThis::add(emailline, emailMsg);

  QWhatsThis::add(apidoc, i18n("Generate a HTML-based documentation set\n"
             "for your project classes including cross-references to\n"
             "the used libraries."));

  QWhatsThis::add(userdoc, i18n("Generate a preset documentation handbook in HTML\n"
        "by an SGML file included with your package."));

  QWhatsThis::add(lsmfile, i18n("Create a lsm-file for your project. The Linux Software Map\n"
        "is a file generally used for projects for distribution purpose and contains\n"
        "a short description of the project including the requirements on the side of\n"
        "the end-user."));

  QWhatsThis::add(gnufiles, i18n("Generate a set of GNU standard files for your project.\n"
         "These will give the end-user of the sourcepackage an overview about\n"
         "the licensing, readme's etc, as well as a ChangeLog file for you to\n"
         "protocol your changes."));
  QWhatsThis::add(progicon, i18n("Add a program icon to your project that represents\n"
         "your application in the window manager."));
  QWhatsThis::add(miniicon, i18n("Add a mini program icon to your project that is used\n"
         "for window-manager popup menus."));
  QWhatsThis::add(datalink, i18n("Add a KDE link file which is installed in\n"
         "the KDE panel of the end-user. By default, your application's\n"
         "link file will be installed in the Applications-menu. You can\n"
         "change this destination by editing the installation properties\n"
         "for the link file later."));
  QWhatsThis::add(iconload, i18n("Lets you select another icon for your program\n"
         "than the sample program icon provided by the Application\n"
         "Wizard."));
  QWhatsThis::add(miniload, i18n("Lets you select another mini-icon for your program\n"
         "than the sample program icon provided by the Application\n"
         "Wizard."));

  connect(nameline,SIGNAL(textChanged(const QString&)),SLOT(slotProjectnameEntry(const QString&)));
  connect(directoryline,SIGNAL(textChanged(const QString&)),SLOT(slotDirectoryEntry(const QString&)));
  connect(directoryload,SIGNAL(clicked()),SLOT(slotDirDialogClicked()));
  connect(miniload,SIGNAL(clicked()),SLOT(slotMiniIconButtonClicked()));
  connect(progicon,SIGNAL(clicked()),SLOT(slotProgIconClicked()));
  connect(miniicon,SIGNAL(clicked()),SLOT(slotMiniIconClicked()));
  connect(iconload,SIGNAL(clicked()),SLOT(slotIconButtonClicked()));

  /************************************************************/

  // create the thirth page
  page2 = new QWidget(this);
  page2->setEnabled(true);
  QGridLayout *grid1 = new QGridLayout(page2,2,2,15,7);

  addPage(page2, i18n("Version Control System Support"));
  setFinishEnabled ( page2, true );

  qtarch_ButtonGroup_1 = new QButtonGroup( page2, "ButtonGroup_1" );
  //  qtarch_ButtonGroup_1->setGeometry( 20, 50, 460, 360 );
  //  qtarch_ButtonGroup_1->setMinimumSize( 0, 0 );
  //  qtarch_ButtonGroup_1->setMaximumSize( 32767, 32767 );
  qtarch_ButtonGroup_1->setFocusPolicy( QWidget::NoFocus );
  qtarch_ButtonGroup_1->setBackgroundMode( QWidget::PaletteBackground );
  qtarch_ButtonGroup_1->setFrameStyle( 49 );
  qtarch_ButtonGroup_1->setAlignment( 1 );
  grid1->addMultiCellWidget(qtarch_ButtonGroup_1, 1, 1, 0, 1);

  vsSupport = new QLabel( page2, "vsSupport" );
  vsSupport->setFocusPolicy( QWidget::NoFocus );
  vsSupport->setBackgroundMode( QWidget::PaletteBackground );
  vsSupport->setText(i18n( "VCS Support" ));
  vsSupport->setAlignment( 289 );
  vsSupport->setMargin( -1 );
  grid1->addWidget(vsSupport,0,0);

  vsBox = new QComboBox( FALSE, page2, "vsBox" );
  vsBox->setFocusPolicy( QWidget::StrongFocus );
  vsBox->setBackgroundMode( QWidget::PaletteBackground );
  vsBox->setSizeLimit( 10 );
  vsBox->setAutoResize( FALSE );
  vsBox->insertItem( i18n("NONE") );
  vsBox->insertItem( i18n("CVS") );
  grid1->addWidget(vsBox,0,1);

  QGridLayout *grid2 = new QGridLayout( qtarch_ButtonGroup_1,5,3,15,7);


  vsInstall = new QLabel(  qtarch_ButtonGroup_1, "vsInstall" );
  vsInstall->setFocusPolicy( QWidget::NoFocus );
  vsInstall->setBackgroundMode( QWidget::PaletteBackground );
  vsInstall->setText(i18n( "VCS Location") );
  vsInstall->setAlignment( 289 );
  vsInstall->setMargin( -1 );
  grid2->addWidget(vsInstall,0,0);



  vsLocation = new QLineEdit(qtarch_ButtonGroup_1, "vsLocation" );
  vsLocation->setFocusPolicy( QWidget::StrongFocus );
  vsLocation->setBackgroundMode( QWidget::PaletteBase );
  vsLocation->setText( "" );
  vsLocation->setMaxLength( 32767 );
  vsLocation->setEchoMode( QLineEdit::Normal );
  vsLocation->setFrame( TRUE );
  grid2->addWidget(vsLocation,0,1);

  locationbutton = new QPushButton( qtarch_ButtonGroup_1, "locationbutton" );
  locationbutton->setFocusPolicy( QWidget::TabFocus );
  locationbutton->setBackgroundMode( QWidget::PaletteBackground );
  locationbutton->setPixmap(pix);
  locationbutton->setAutoRepeat( FALSE );
  locationbutton->setAutoResize( FALSE );
  grid2->addWidget(locationbutton,0,2);


  projectVSLocation = new QLabel( qtarch_ButtonGroup_1, "projectVSLocation" );
  projectVSLocation->setFocusPolicy( QWidget::NoFocus );
  projectVSLocation->setBackgroundMode( QWidget::PaletteBackground );
  projectVSLocation->setText(i18n( "Repository in VCS") );
  projectVSLocation->setAlignment( 289 );
  projectVSLocation->setMargin( -1 );
  grid2->addWidget(projectVSLocation,1,0);


  projectlocationline = new QLineEdit( qtarch_ButtonGroup_1, "projectlocationline" );
  projectlocationline->setFocusPolicy( QWidget::StrongFocus );
  projectlocationline->setBackgroundMode( QWidget::PaletteBase );
  projectlocationline->setText( "" );
  projectlocationline->setMaxLength( 32767 );
  projectlocationline->setEchoMode( QLineEdit::Normal );
  projectlocationline->setFrame( TRUE );
  grid2->addWidget(projectlocationline,1,1);



  vendorTag = new QLabel(qtarch_ButtonGroup_1 , "vendorTag" );
  vendorTag->setFocusPolicy( QWidget::NoFocus );
  vendorTag->setBackgroundMode( QWidget::PaletteBackground );
  vendorTag->setText(i18n( "Vendor Tag") );
  vendorTag->setAlignment( 289 );
  vendorTag->setMargin( -1 );
  grid2->addWidget(vendorTag,2,0);

  vendorline = new QLineEdit( qtarch_ButtonGroup_1, "vendorline" );
  vendorline->setFocusPolicy( QWidget::StrongFocus );
  vendorline->setBackgroundMode( QWidget::PaletteBase );
  vendorline->setText( "" );
  vendorline->setMaxLength( 32767 );
  vendorline->setEchoMode( QLineEdit::Normal );
  vendorline->setFrame( TRUE );
  grid2->addWidget(vendorline,2,1);


  logMessage = new QLabel( qtarch_ButtonGroup_1, "logMessage" );
  logMessage->setFocusPolicy( QWidget::NoFocus );
  logMessage->setBackgroundMode( QWidget::PaletteBackground );
  logMessage->setText(i18n( "Log Message") );
  logMessage->setAlignment( 289 );
  logMessage->setMargin( -1 );
  grid2->addWidget(logMessage,3,0);



  messageline = new QLineEdit( qtarch_ButtonGroup_1, "messageline" );
  messageline->setFocusPolicy( QWidget::StrongFocus );
  messageline->setBackgroundMode( QWidget::PaletteBase );
  messageline->setText( "" );
  messageline->setMaxLength( 32767 );
  messageline->setEchoMode( QLineEdit::Normal );
  messageline->setFrame( TRUE );
  grid2->addWidget( messageline,3,1);


  releaseTag = new QLabel( qtarch_ButtonGroup_1, "releaseTag" );
  releaseTag->setFocusPolicy( QWidget::NoFocus );
  releaseTag->setBackgroundMode( QWidget::PaletteBackground );
  releaseTag->setText(i18n( "Release Tag") );
  releaseTag->setAlignment( 289 );
  releaseTag->setMargin( -1 );
  grid2->addWidget( releaseTag,4,0);


  releaseline = new QLineEdit( qtarch_ButtonGroup_1, "releaseline" );
  releaseline->setFocusPolicy( QWidget::StrongFocus );
  releaseline->setBackgroundMode( QWidget::PaletteBase );
  releaseline->setText( "" );
  releaseline->setMaxLength( 32767 );
  releaseline->setEchoMode( QLineEdit::Normal );
  releaseline->setFrame( TRUE );
  grid2->addWidget( releaseline,4,1);

  qtarch_ButtonGroup_1->insert( locationbutton );

  connect(locationbutton,SIGNAL(clicked()),SLOT(slotLocationButtonClicked()));
  connect(vsBox,SIGNAL(activated(int)),SLOT(slotVSBoxChanged(int)));
  connect(vendorline,SIGNAL(textChanged(const QString&)),SLOT(slotVendorEntry(const QString&)));

  projectlocationline->setEnabled(false);

  QString vcLogMsg = i18n("Here you can enter the log message for the\n"
               "version control system.");
  QWhatsThis::add(messageline, vcLogMsg);
  QWhatsThis::add(logMessage, vcLogMsg);

  QString vcVendorMsg = i18n("Here you can choose the vendor tag, which your project\n"
                              "has in the version control system.");
  QWhatsThis::add(vendorTag, vcVendorMsg);
  QWhatsThis::add(vendorline, vcVendorMsg);

  QString vcReleaseMsg = i18n("Here you can choose a special pointer for the first\n"
               "entry in the version control system.");
  QWhatsThis::add(releaseTag, vcReleaseMsg);
  QWhatsThis::add(releaseline, vcReleaseMsg);

  QString vcVsSupportMsg = i18n("Here you can choose the version control system,\n"
               "which you want to use.");
  QWhatsThis::add(vsBox, vcVsSupportMsg);
  QWhatsThis::add(vsSupport, vcVsSupportMsg);

  QString vcVsLocationMsg = i18n("Here you can choose where your vcsroot loction should be.\n"
                 "At the moment we only support local vs. And be sure, you\n"
                 "have read and write access in the location.");
  QWhatsThis::add(vsInstall, vcVsLocationMsg);
  QWhatsThis::add(vsLocation, vcVsLocationMsg);
  QWhatsThis::add(locationbutton, vcVsLocationMsg);

  QString projectVSLocationMsg = i18n("Here you can see the repository of your project in the\n"
               "version control system. The repository is dependend on\n"
               "the directory of your project. You can not change the\n"
               "repository direct.");
  QWhatsThis::add(projectVSLocation, projectVSLocationMsg);
  QWhatsThis::add(projectlocationline,  projectVSLocationMsg);


  /************************************************************/

  // create the forth page
  page3 = new QWidget(this);
  page3->setEnabled(true);
  addPage(page3, i18n("Headertemplate for .h-files"));
  grid1 = new QGridLayout(page3,2,3,15,7);

  setFinishEnabled ( page3, true );

  hheader = new QCheckBox( page3, "hheader" );
  hheader->setFocusPolicy( QWidget::TabFocus );
  hheader->setBackgroundMode( QWidget::PaletteBackground );
  hheader->setText( i18n("headertemplate for .h-files") );
  hheader->setAutoRepeat( FALSE );
  hheader->setAutoResize( FALSE );
  grid1->addWidget(hheader,0,0);

  hload = new QPushButton( page3, "hload" );
  hload->setFocusPolicy( QWidget::TabFocus );
  hload->setBackgroundMode( QWidget::PaletteBackground );
  hload->setText(i18n( "Load..." ));
  hload->setAutoRepeat( FALSE );
  hload->setAutoResize( FALSE );
  grid1->addWidget(hload,0,1);


  hnew = new QPushButton( page3, "hnew" );
  hnew->setFocusPolicy( QWidget::TabFocus );
  hnew->setBackgroundMode( QWidget::PaletteBackground );
  hnew->setText( i18n("New" ));
  hnew->setAutoRepeat( FALSE );
  hnew->setAutoResize( FALSE );
  grid1->addWidget(hnew,0,2);

  hedit = new KEdit(page3 );
  QFont f("fixed",10);
  hedit->setFont(f);
  hedit->setFocusPolicy( QWidget::StrongFocus );
  hedit->setBackgroundMode( QWidget::PaletteBase );
  hedit->insertLine( "" );
  hedit->setReadOnly( FALSE );
  hedit->setOverwriteMode( FALSE );
  grid1->addMultiCellWidget(hedit,1,1,0,2);

  QToolTip::add(hload,i18n("Press this button to select an\n"
         "existing header template file"));
  QToolTip::add(hnew,i18n("Clears the pre-set headertemplate"));
  QToolTip::add(hedit,i18n("Edit your headertemplate here"));

  QWhatsThis::add(hheader, i18n("Use a standard\n"
        "headertemplate for your headerfiles"));


  connect(hheader,SIGNAL(clicked()),SLOT(slotHeaderHeaderClicked()));
  connect(hload,SIGNAL(clicked()),SLOT(slotHeaderDialogClicked()));
  connect(hnew,SIGNAL(clicked()),SLOT(slotNewHeaderButtonClicked()));

  /************************************************************/

  // create the fifth page
  page4 = new QWidget(this);
  page4->setEnabled(true);
  addPage(page4, i18n("Headertemplate for .cpp-files"));
  setFinishEnabled ( page4, true );
  grid1 = new QGridLayout(page4,2,3,15,7);


  cppheader = new QCheckBox( page4, "cppheader" );
  cppheader->setFocusPolicy( QWidget::TabFocus );
  cppheader->setBackgroundMode( QWidget::PaletteBackground );
  cppheader->setText( i18n("headertemplate for .cpp-files") );
  cppheader->setAutoRepeat( FALSE );
  cppheader->setAutoResize( FALSE );
  grid1->addWidget(cppheader,0,0);

  cppload = new QPushButton( page4, "cppload" );
  cppload->setFocusPolicy( QWidget::TabFocus );
  cppload->setBackgroundMode( QWidget::PaletteBackground );
  cppload->setText( i18n("Load...") );
  cppload->setAutoRepeat( FALSE );
  cppload->setAutoResize( FALSE );
  grid1->addWidget(cppload,0,1);

  cppnew = new QPushButton( page4, "cppnew" );
  cppnew->setFocusPolicy( QWidget::TabFocus );
  cppnew->setBackgroundMode( QWidget::PaletteBackground );
  cppnew->setText( i18n("New") );
  cppnew->setAutoRepeat( FALSE );
  cppnew->setAutoResize( FALSE );
  grid1->addWidget(cppnew,0,2);

  cppedit = new KEdit(page4);
  cppedit->setFont(f);
  cppedit->setFocusPolicy( QWidget::StrongFocus );
  cppedit->setBackgroundMode( QWidget::PaletteBase );
  cppedit->insertLine( "" );
  cppedit->setReadOnly( FALSE );
  cppedit->setOverwriteMode( FALSE );
  grid1->addMultiCellWidget(cppedit,1,1,0,2);

  QToolTip::add(cppload,i18n("Press this button to select an\n"
           "existing header template file"));
  QToolTip::add(cppnew,i18n("Clears the pre-set headertemplate"));
  QToolTip::add(cppedit,i18n("Edit your headertemplate here"));

  QWhatsThis::add(cppheader, i18n("Use a standard\n"
          "headertemplate for your implementation files"));


  connect(cppheader,SIGNAL(clicked()),SLOT(slotCppHeaderClicked()));
  connect(cppload,SIGNAL(clicked()),SLOT(slotCppDialogClicked()));
  connect(cppnew,SIGNAL(clicked()),SLOT(slotNewCppButtonClicked()));

  /************************************************************/

  // create the sixth page
  page5 = new QWidget(this);
  page5->setEnabled(true);
  addPage(page5, i18n("Processes"));
  setFinishEnabled ( page5, true );
  setNextEnabled( page5, false );
  grid1 = new QGridLayout(page5,7,1,15,0);

  // create a MultiLineEdit for the processe of kAppWizard
  output = new QMultiLineEdit( page5, "output" );
  output->setFocusPolicy( QWidget::StrongFocus );
  output->setBackgroundMode( QWidget::PaletteBase );
  output->insertLine( "" );
  output->setReadOnly( TRUE );
  output->setOverwriteMode( FALSE );
  grid1->addMultiCellWidget(output,0,4,0,0);

  errOutput = new QMultiLineEdit( page5, "errOutput" );
  errOutput->setFocusPolicy( QWidget::StrongFocus );
  errOutput->setBackgroundMode( QWidget::PaletteBase );
  errOutput->insertLine( "" );
  errOutput->setReadOnly( TRUE );
  errOutput->setOverwriteMode( FALSE );
  QFont font("helvetica",10);
  output->setFont(font);
  grid1->addMultiCellWidget(errOutput,5,6,0,0);
  QToolTip::add(output,i18n("Displays the normal output of the project generator"));
  errOutput->setFont(font);
  QToolTip::add(errOutput,i18n("Displays warnings and errormessages of the project generator"));
}

// connection to directoryload
void CKAppWizard::slotDirDialogClicked()
{
  QString projname = nameline->text();
  QString dirPath = KFileDialog::getExistingDirectory(m_dir);
  if (modifyDirectory)
    m_dir = dirPath;
  else
    m_dir = dirPath + projname.lower();

  directoryline->setText(m_dir);
  m_dir = dirPath;
}

// connection of hload
void CKAppWizard::slotHeaderDialogClicked()
{
  QString fileName = KFileDialog::getOpenFileName(QDir::homeDirPath(),
                                        "*",
                                        0,
                                        i18n("Select your template for Header-file headers"));

  if (!fileName.isEmpty())
  {
    QFile fileIODev(fileName);
    if (fileIODev.open(IO_ReadOnly))
    {
      QTextStream textStream(&fileIODev);
      hedit->clear();
      hedit->insertText(&textStream);
    }
  }
}

// connection of cppload
void CKAppWizard::slotCppDialogClicked()
{
  QString fileName = KFileDialog::getOpenFileName(QDir::homeDirPath(),
                                        "*",
                                        0,
                                        (citem->isSelected())
                                            ? i18n("Select your template for C-file headers")
                                            : i18n("Select your template for Cpp-file headers"));
  if (!fileName.isEmpty())
  {
    QFile fileIODev(fileName);
    if (fileIODev.open(IO_ReadOnly))
    {
      QTextStream textStream(&fileIODev);
      cppedit->clear();
      cppedit->insertText(&textStream);
    }
  }
}

// connection of hnew
void CKAppWizard::slotNewHeaderButtonClicked() {
  hedit->clear();
}

// connection of cppnew
void CKAppWizard::slotNewCppButtonClicked() {
  cppedit->clear();
}

// connection of this (defaultButton)
void CKAppWizard::accept() {

  if (!(CToolClass::searchInstProgram("sgml2html") || CToolClass::searchInstProgram("ksgml2html"))) {
    userdoc->setChecked(false);
    KMessageBox::error (0,
                    i18n("If you want to generate the user-documentation, you need one of these programs."),
                    i18n("sgml2html and ksgml2html do not exist!"));
  }


  if (vsBox->currentItem() == 1) {
    if (!CToolClass::searchProgram("cvs")) {
      return;
    }
  }
  QDir dir;
  QString direct = directoryline->text();

  int pos;
  // shouldn't happen but who knows... maybe there are some people who
  //  will leave this field empty, so root will be assumed...
  if (direct.isEmpty())
    direct="/";
  if (direct.right(1) == "/" && direct.length()>1)
    direct=direct.left(direct.length()-1);
  if ((pos=direct.findRev('/'))>1)
    direct=direct.left(pos);
  dir.setPath(direct);


  if (!dir.exists()) {
    KShellProcess p("/bin/sh");
    p.clearArguments();
    p << "mkdirhier";
    p << direct;
    p.start(KProcess::Block,KProcess::AllOutput);
  }

  if (!QFileInfo(direct).isWritable())
  {
    KMessageBox::error(this,  i18n("Either the desired directory cannot be created or\n"
                                    "you haven't enough rights to use it.\n"
                                    "It isn't possible to generate a new project into this directory."),
                              i18n("%1 isn't writable!").arg(direct));
    return;
  }

  // Error if project directory already exists
  dir.setPath(directoryline->text());
  if (dir.exists()) {
    KMessageBox::error( this, i18n("It isn't possible to generate a new project into an existing directory."),
                        i18n("%1 already exists!").arg(directoryline->text()));
      return;

  }
  else {
    okPermited();
  }
}

/* This function creates the default configure arguments for the actual project */

QString CKAppWizard::createConfigureArgs() 
{

  KConfig *config = KGlobal::config();
  config->setGroup("QT2");
  QString qtpath=config->readEntry("qt2dir");
  QString kde2path=config->readEntry("kde2dir");
  QString args(""), kdeargs(""), qtargs("");

  if (/*!sharedlibitem->isSelected() && */ !gnomenormalitem->isSelected() &&
      !citem->isSelected() && !cppitem->isSelected() && 
      !customprojitem->isSelected())
  {
    if (!kde2path.isEmpty())
    {
      if(kde2path.right(1) == "/")
          kde2path=kde2path.remove(kde2path.length()-1,1);
      kdeargs="--prefix="+kde2path;   
    }
    
    if (!qtpath.isEmpty())
    {
      if(qtpath.right(1) == "/")
          qtpath=qtpath.remove(qtpath.length()-1,1);
      qtargs="--with-qt-dir="+qtpath;
    }
     
    if( qextmdiitem->isSelected())
    {
      if (!qtargs.isEmpty())
        qtargs+=" ";
      qtargs+="--enable-kde=no";
    }
      
    if(qt2normalitem->isSelected() || qt2mdiitem->isSelected() || qextmdiitem->isSelected())
       args=qtargs;
    else
    {
      if (!kdeargs.isEmpty() && !qtargs.isEmpty())
        kdeargs+=" ";
      args=kdeargs+qtargs;
    }
  }
  
  return args;  
}

void CKAppWizard::generateEntries(const QString &filename) {

  QString entriesfilename(filename.isEmpty() ? QString("entries") : filename);
  QString docu_dir, index_path, libname, link;
  KConfig* config=KGlobal::config();
  config->setGroup("Doc_Location");
  docu_dir = config->readEntry("doc_kde", KDELIBS_DOCDIR);
  if (!docu_dir.isEmpty())
  {
    index_path= docu_dir + "/kdoc-reference";
    QDir d;
    d.setPath(index_path);
    if(d.exists()){
      const QFileInfoList *fileList = d.entryInfoList(); // get the file info list
      QFileInfoListIterator it( *fileList ); // iterator
      QFileInfo *fi; // the current file info
      while ( (fi=it.current()) ) {  // traverse all kdoc reference files
        libname=fi->fileName();  // get the filename
        if(fi->isFile())
        {
         if (fi->baseName() != QString("libkmid")) 
         { // workaround for a strange behaviour of kdoc: don't try libkmid
          libname=" -l"+fi->baseName();  // get only the base of the filename as library name
          link+=libname;
         }
        }
        ++it; // increase the iterator
      }
    }
    else
      index_path="";
  }

  // Create filename to open in a secure manner

  // Open file and attach stream
  QFile entriesfile( locateLocal("appdata", entriesfilename));

  if ( entriesfile.open(IO_ReadWrite) )
  {
    QTextStream entries( &entriesfile );
    entries << "TEMPLATESDIR\n";
    entries << KStandardDirs::kde_default("data") + "kdevelop/templates";
    entries << "\nKDEICONDIR\n";
    entries << KStandardDirs::kde_default("icon");
    entries << "\nAPPLICATION\n";

    if (kde2miniitem->isSelected()) {
      entries << "kde2mini\n";
    }
    else if (kde2normalitem->isSelected()) {
      entries << "kde2normal\n";
    }
    else if (kde2mdiitem->isSelected()) {
      entries << "kde2mdi\n";
    }
    else if (qt2normalitem->isSelected()) {
       entries << "qt2normal\n";
    }
    else if (qt2mdiitem->isSelected()) {
       entries << "qt2mdi\n";
    }
    else if (qextmdiitem->isSelected()) {
       entries << "qextmdi\n";
    }
    else if (cppitem->isSelected()) {
      entries << "cpp\n";
    }
    else if (citem->isSelected()) {
      entries << "c\n";
    }
    else if (gnomenormalitem->isSelected()) {
      entries << "gnomenormal\n";
    }
    /*
      else if (gtkminiitem->isSelected()) {
      entries << "gtkmini\n";
      }*/
    else if (kickeritem->isSelected()) {
      entries << "kickerapp\n";
    }
    else if (kioslaveitem->isSelected()) {
      entries << "kioslave\n";
    }
    else if (kthemeitem->isSelected()) {
      entries << "ktheme\n";
    }
    else if (kcmoduleitem->isSelected()) {
      entries << "kcmodule\n";
    }
//    else if (sharedlibitem->isSelected()) {
//      entries << "sharedlib\n";
//    }
    else if (kpartitem->isSelected()) {
      entries << "kpart\n";
    }
    else if (customprojitem->isSelected()) {
      entries << "customproj\n";
    }

    entries << "CONFIGARG\n";
    entries << createConfigureArgs() << "\n";
      
    entries << "NAME\n";
    entries << nameline->text() << "\n";
    entries << "DIRECTORY\n";

    QString direct = directoryline->text();
    if (direct.right(1) == "/") {
      direct = direct.left(direct.length() - 1);
      /* }
         int pos;
         pos = direct.findRev ("/");
         direct = direct.left (pos);
         if(direct.right(1) == "/"){
         entries << direct << "\n";*/
    }
    //       else{
    //       entries << direct << "/\n";
    //       }
    entries << direct << "\n";

    entries << "AUTHOR\n";
    entries << authorline->text() << "\n";
    entries << "EMAIL\n";
    entries << emailline->text() << "\n";
    entries << "API\n";
    if (apidoc->isChecked())
      entries << "yes\n";
    else entries << "no\n";
    
    entries << "KDE_QTVER\n";
    entries << ((KDE_QTVER==3) ? "3\n" : "2\n");
    
    entries << "KDOC_CALL\n";

    if (!index_path.isEmpty() && !link.isEmpty())
      index_path=QString(" -L")+index_path+link;

    bool bCreateKDoc;

    config->setGroup("General Options");
    bCreateKDoc = config->readBoolEntry("CreateKDoc", false);
    if (bCreateKDoc)
     entries << QString("kdoc -p -d '|UNDERDIRECTORY|-api'")+
    index_path+" -n "+nameline->text()+" *.h\n";
    else
     entries << QString("kdoc -p -d '|UNDERDIRECTORY|-api'")+
    index_path+" *.h\n";

    entries << "XGETTEXT\n";
    if (CToolClass::searchProgram("xgettext"))
      entries << "yes\n";
    else entries << "no\n";
    entries << "USER\n";
    if (userdoc->isChecked())
      entries << "yes\n";
    else entries << "no\n";
    entries << "LSM\n";
    if (lsmfile->isChecked())
      entries << "yes\n";
    else entries << "no\n";
    entries << "GNU\n";
    if (gnufiles->isChecked())
      entries << "yes\n";
    else entries << "no\n";
    entries << "PROGICON\n";

    if (name1.isNull())
      name1 = "";
    if (progicon->isChecked())
      entries << name1 << "\n";
    else
      entries << "no\n";

    entries << "MINIICON\n";
    if (name2.isNull())
      name2 = "";
    if (miniicon->isChecked())
      entries << name2 << "\n";
    else
      entries << "no\n";
    entries << "KDELNK\n";
    if (datalink->isChecked())
      entries << "yes\n";
    else entries << "no\n";
    entries << "HEADER\n";
    if (hheader->isChecked())
      entries << "yes\n";
    else entries << "no\n";
    entries << "CPP\n";
    if (cppheader->isChecked())
      entries << "yes\n";
    else entries << "no\n";
    entries << "VERSION\n";
    entries << versionline->text() << "\n";
    entries << "VSSUPPORT\n";
    if (vsBox->currentItem()==0)
      entries << "none\n";          // do not translated
    else
      entries << QString(vsBox->text(vsBox->currentItem())).lower() + "\n";

    entries << "VENDORTAG\n";
    entries << QString(vendorline->text()) + "\n";
    entries << "RELEASETAG\n";
    entries << QString(releaseline->text()) + "\n";
    entries << "VSLOCATION\n";
    entries << QString(vsLocation->text()) + "\n";
    entries << "PRJVSLOCATION\n";
    entries << QString(projectlocationline->text()) + "\n";
    entries << "LOGMESSAGE\n";
    entries << QString(messageline->text()) + "\n";


  entriesfile.flush();
  entriesfile.close();

  } // if (file open)
  else
  {
    debug( "ERROR: EntriesFile not open!" );
    exit( 1 );
  } // end if

}

void CKAppWizard::okPermited()
{
//  m_cancelButton->setFixedWidth(75);
  m_cancelButton->setEnabled(false);
  m_defaultButton->setEnabled(false);

  m_cancelButton->setText(i18n("Exit"));
  errOutput->clear();
  output->clear();

  QString outPath=locateLocal("appdata","");
  KStandardDirs::makeDir(outPath);

  QFile cppFile(outPath+"/cpp");
  if (cppFile.open(IO_WriteOnly))
  {
    QString cppStr = cppedit->text();
    QTextStream textStream(&cppFile);
    textStream << cppStr;
  }
  cppFile.flush();
  cppFile.close();

  QFile headerFile(outPath+"/header");
  if (headerFile.open(IO_WriteOnly))
  {
    QString heditStr = hedit->text();
    QTextStream textStream(&headerFile);
    textStream << heditStr;
  }
  headerFile.flush();
  headerFile.close();

  // making the entries-filename unique... so two Kdevelops on the same
  //  account can let run the ApplicationWizard concurrent
  entriesfname.sprintf("entries.%lX", (long) parent());
  generateEntries(entriesfname);

  namelow = nameline->text();
  namelow = namelow.lower();

  QString prjdir = directoryline->text();
  QDir dir;

  dir.mkdir(prjdir);
  if (!dir.exists())
    dir.setCurrent(prjdir);

  KShellProcess p("/bin/sh");
  QString copysrc;
  QString copydes;
  QString vcsInit;
  if (vsBox->currentItem() == 1) {
    copydes = locateLocal("appdata", "kdeveloptemp"); //QDir::homeDirPath() + "/.kde/share/apps/kdevelop/kdeveloptemp";
    dir.mkdir(copydes);
    dir.setCurrent(copydes);

    vcsInit = (QString) "cvs -d " + vsLocation->text() + (QString) " init";
    p.clearArguments();
    p << vcsInit;
    p.start(KProcess::Block,KProcess::AllOutput);

  }
  else {
    copydes = prjdir;
  }

  QString admindes = copydes + "/admin.tar.gz";
  QString adminsrc = locate("appdata", "templates/admin.tar.gz");
  copydes += "/template.tar.gz";

  p.clearArguments();
  bool hasTemplate = true;
  if (kde2miniitem->isSelected()) {
    copysrc = locate("appdata", "templates/mini2.tar.gz");
  }
  else if (kde2normalitem->isSelected()) {
    copysrc = locate("appdata", "templates/normal2.tar.gz");
  }
  else if (kde2mdiitem->isSelected()) {
    copysrc = locate("appdata", "templates/kdemdi.tar.gz");
  }
  else if (qt2normalitem->isSelected()) {
    copysrc = locate("appdata", "templates/qt2.tar.gz");
  }
  else if (qt2mdiitem->isSelected()) {
    copysrc = locate("appdata", "templates/qtmdi.tar.gz");
  }
  else if (qextmdiitem->isSelected()) {
    copysrc = locate("appdata", "templates/qextmdi.tar.gz");
  }
  else if (cppitem->isSelected()) {
    copysrc = locate("appdata", "templates/cpp.tar.gz");
  }
  else if (citem->isSelected()) {
    copysrc = locate("appdata", "templates/c.tar.gz");
  }
  else if (gnomenormalitem->isSelected()) {
    copysrc = locate("appdata", "templates/gnome.tar.gz");
  }
  else if (kickeritem->isSelected()) {
    copysrc = locate("appdata", "templates/kicker.tar.gz");
  }
  else if (kpartitem->isSelected()) {
    copysrc = locate("appdata", "templates/kpart.tar.gz");
  }
  else if (kioslaveitem->isSelected()) {
    copysrc = locate("appdata", "templates/kioslave.tar.gz");
  }
  else if (kthemeitem->isSelected()) {
    copysrc = locate("appdata", "templates/ktheme.tar.gz");
  }
  else if (kcmoduleitem->isSelected()) {
    copysrc = locate("appdata", "templates/kcmodule.tar.gz");
  }
//  else if (sharedlibitem->isSelected()) {
//    copysrc = locate("appdata", "templates/sharedlib.tar.gz");
//  }
  else {
    hasTemplate = false;
  }

  if (hasTemplate)
  {
    if (QFileInfo(copysrc).exists())
    {
      p << "cp";
      p << "'" + copysrc + "'";
      p << "'" + copydes + "';";

/*    Almost all projects with templates also use the admin-dir (W. Tasin 2001-11-02)
      if( (kthemeitem->isSelected()||kcmoduleitem->isSelected()||kpartitem->isSelected()||kickeritem->isSelected()||kioslaveitem->isSelected()||
			kde2miniitem->isSelected()||kde2normalitem->isSelected()||kde2mdiitem->isSelected()||
			qt2normalitem->isSelected()||qt2mdiitem->isSelected() || qextmdiitem->isSelected()))
*/
      if ( !gnomenormalitem->isSelected() )
      {
        if (QFileInfo(adminsrc).exists())
        {
          p << "cp";
          p << "'" + adminsrc + "'";
          p << "'" + admindes + "'";
        }
        else
        {
          KMessageBox::error (this, QString(i18n("The template file [%1] is missing.\n"
                                  "Please correct your KDevelop installation.")).arg(adminsrc));
          return;
        }
      }

      p.start(KProcess::Block,KProcess::AllOutput);
    }
    else
    {
      KMessageBox::error (this, QString(i18n("The template file [%1] is missing.\n"
                          "Please correct your KDevelop installation.")).arg(copysrc));
      return;
    }
  }

  q->clearArguments();
  connect(q,SIGNAL(processExited(KProcess *)),this,SLOT(slotProcessExited()));
  connect(q,SIGNAL(receivedStdout(KProcess *, char *, int)),
          this,SLOT(slotPerlOut(KProcess *, char *, int)));
  connect(q,SIGNAL(receivedStderr(KProcess *, char *, int)),
          this,SLOT(slotPerlErr(KProcess *, char *, int)));
  *q << "perl" << locate("appdata", "tools/processes.pl");

  if (!entriesfname.isEmpty())
    *q << entriesfname;

  *q << locateLocal("appdata", "");

  q->start(KProcess::NotifyOnExit, KProcess::AllOutput);
  m_finishButton->setEnabled(false);

  showPage(page5);
  page0->setEnabled(false);
  page1->setEnabled(false);
  page2->setEnabled(false);
  page3->setEnabled(false);
  page4->setEnabled(false);

  apidoc->setEnabled(false);
  lsmfile->setEnabled(false);
  cppheader->setEnabled(false);
  hheader->setEnabled(false);
  datalink->setEnabled(false);
  miniicon->setEnabled(false);
  progicon->setEnabled(false);
  gnufiles->setEnabled(false);
  userdoc->setEnabled(false);
  directoryline->setEnabled(false);
  nameline->setEnabled(false);
  m_finishButton->setEnabled(false);
  miniload->setEnabled(false);
  iconload->setEnabled(false);
  cppedit->setEnabled(false);
  hedit->setEnabled(false);
  authorline->setEnabled(false);
  emailline->setEnabled(false);
  versionline->setEnabled(false);
  hnew->setEnabled(false);
  hload->setEnabled(false);
  cppnew->setEnabled(false);
  cppload->setEnabled(false);
  applications->setEnabled(false);
  generatesource->setEnabled(false);
  apphelp->setEnabled(false);
  messageline->setEnabled(false);
  logMessage->setEnabled(false);
  vendorline->setEnabled(false);
  vendorTag->setEnabled(false);
  releaseline->setEnabled(false);
  releaseTag->setEnabled(false);
  vsInstall->setEnabled(false);
  projectVSLocation->setEnabled(false);
  vsLocation->setEnabled(false);
  locationbutton->setEnabled(false);
  qtarch_ButtonGroup_1->setEnabled(false);
  vsBox->setEnabled(false);
  vsSupport->setEnabled(false);
  name->setEnabled(false);
  email->setEnabled(false);
  authorname->setEnabled(false);
  versionnumber->setEnabled(false);
  directory->setEnabled(false);
}


// connection of this (m_cancelButton)
void CKAppWizard::slotAppEnd()
{
  nametext = nameline->text();
  m_author_name = authorline->text();
  m_author_email = emailline->text();

  if ((!( m_finishButton->isEnabled())) && (nametext.length() >= 1)) {

    delete (project);
    project = 0;
  }

  reject();
}

void CKAppWizard::removeSources(const QString &dir)
{
  QString extension= (citem->isSelected()) ? "c" : "cpp";
  nametext = nameline->text();
  nametext = nametext.lower();
  QFile file;
  file.remove (dir + "/" + nametext + "/main."+extension);
  if (!citem->isSelected() && !cppitem->isSelected())
  {
    file.remove (dir + "/" + nametext + "/" + nametext + ".cpp");
    file.remove (dir + "/" + nametext + "/" + nametext + ".h");
  }
  if (kde2normalitem->isSelected()|| kde2mdiitem->isSelected() ||
      qt2normalitem->isSelected()||
      qt2mdiitem->isSelected() || qextmdiitem->isSelected())
  {
    file.remove (dir + "/" + nametext + "/" + nametext + "doc.cpp");
    file.remove (dir + "/" + nametext + "/" + nametext + "doc.h");
    file.remove (dir + "/" + nametext + "/" + nametext + "view.cpp");
    file.remove (dir + "/" + nametext + "/" + nametext + "view.h");
    if(qextmdiitem->isSelected())
      file.remove (dir + "/" + nametext + "/resource.h");
  }
  if( qextmdiitem->isSelected())
  {
    file.remove (dir + "/" + nametext + "/tabprocessingeditwidget.cpp");
    file.remove (dir + "/" + nametext + "/tabprocessingeditwidget.h");
  }
  if( gnomenormalitem->isSelected()){
    file.remove (dir + "/" + nametext + "/main.c");
    file.remove (dir + "/" + nametext + "/main.h");
    file.remove (dir + "/" + nametext + "/menus.c");
    file.remove (dir + "/" + nametext + "/menus.h");
    file.remove (dir + "/" + nametext + "/app.c");
    file.remove (dir + "/" + nametext + "/app.h");
  }

  if( kickeritem->isSelected() ||kpartitem->isSelected()||kioslaveitem->isSelected()||
  		kthemeitem->isSelected()||kcmoduleitem->isSelected()/*||sharedlibitem->isSelected()*/)
 	{
      file.remove(dir + "/" + nametext + "/main.cpp");
	}
}
// connection of this ( m_finishButton)
void CKAppWizard::slotPerlOut(KProcess*,char* buffer,int buflen) {
  QCString str(buffer,buflen);
  output->append(str);
  output->setCursorPosition(output->numLines(),0);
}

// connection of this ( m_finishButton)
void CKAppWizard::slotPerlErr(KProcess*,char* buffer,int buflen) {
  QCString str(buffer,buflen);
  errOutput->append(str);
  errOutput->setCursorPosition(errOutput->numLines(),0);
}

void CKAppWizard::slotApplicationClicked() {
  // reset some titles
  setTitle(page4, i18n("Headertemplate for .cpp-files"));
  cppheader->setText( i18n("headertemplate for .cpp-files") );

  apidoc->setEnabled(!citem->isSelected() && !customprojitem->isSelected());
  userdoc->setEnabled(!customprojitem->isSelected());

  if (kde2normalitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/normalApp.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(true);
    datalink->setEnabled(true);
    datalink->setChecked(true);
    progicon->setEnabled(true);
    progicon->setChecked(true);
    miniicon->setEnabled(true);
    miniicon->setChecked(true);
    miniload->setEnabled(true);
    iconload->setEnabled(true);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a KDE application with session-management, "
         "menubar, toolbar, statusbar and support for a "
         "document-view codeframe model."));
  }
  else if (kde2miniitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/miniApp.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(true);
    datalink->setEnabled(true);
    datalink->setChecked(true);
    progicon->setEnabled(true);
    progicon->setChecked(true);
    miniicon->setEnabled(true);
    miniicon->setChecked(true);
    miniload->setEnabled(true);
    iconload->setEnabled(true);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a KDE application with an empty main widget."));
  }
  else if (kde2mdiitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/kdemdi.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(true);
    datalink->setEnabled(true);
    datalink->setChecked(true);
    progicon->setEnabled(true);
    progicon->setChecked(true);
    miniicon->setEnabled(true);
    miniicon->setChecked(true);
    miniload->setEnabled(true);
    iconload->setEnabled(true);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a KDE MDI (Multiple Document Interface) application with "
         "menubar, toolbar, statusbar and support for a "
         "document-view codeframe model."));
  }
//  else if (sharedlibitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
//  {
//    pm.load(locate("appdata", "pics/sharelib.png"));
//    widget1b->setBackgroundPixmap(pm);
//    apidoc->setChecked(true);
//    datalink->setEnabled(false);
//    datalink->setChecked(false);
//    progicon->setEnabled(false);
//    progicon->setChecked(false);
//    miniicon->setEnabled(false);
//    miniicon->setChecked(false);
//    miniload->setEnabled(false);
//    iconload->setEnabled(false);
//    lsmfile->setChecked(true);
//    gnufiles->setChecked(true);
//    userdoc->setChecked(true);
//    generatesource->setChecked(true);
//    generatesource->setEnabled(true);
//    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
//       m_finishButton->setEnabled(true);
//    }
//    apphelp->setText (i18n("Create a C++ based shared library."));
//  }
  else if (kioslaveitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/kioslave.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(true);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(false);
    progicon->setChecked(false);
    miniicon->setEnabled(false);
    miniicon->setChecked(false);
    miniload->setEnabled(false);
    iconload->setEnabled(false);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a KDE KIO Slave.\n"
                      "KIOSlaves are the foundation for all protocols in KDE2."));
  }
  else if (kpartitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/kpart.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(true);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(false);
    progicon->setChecked(false);
    miniicon->setEnabled(true);
    miniicon->setChecked(true);
    miniload->setEnabled(true);
    iconload->setEnabled(false);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a KDE KPart Plugin. \nTo create a generic plugin for the Konqeror web browser use this template.  This template can also be modified to create generic plugins."));
  }
  else if (kickeritem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/kicker.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(true);
    datalink->setEnabled(true);
    datalink->setChecked(true);
    progicon->setEnabled(true);
    progicon->setChecked(true);
    miniicon->setEnabled(false);
    miniicon->setChecked(false);
    miniload->setEnabled(false);
    iconload->setEnabled(true);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a KDE kicker applet.  \nThese are applets for KDE2's panel."));
  }
  else if (kcmoduleitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/kcmodule.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(true);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(false);
    progicon->setChecked(false);
    miniicon->setEnabled(false);
    miniicon->setChecked(false);
    miniload->setEnabled(false);
    iconload->setEnabled(false);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a KDE Control Center Module.\n"
    									"This template enables you to write your own modules"
    									"to add new system-wide configuration dialogs."));
  }
  else if (kthemeitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/ktheme.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(true);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(false);
    progicon->setChecked(false);
    miniicon->setEnabled(false);
    miniicon->setChecked(false);
    miniload->setEnabled(false);
    iconload->setEnabled(false);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(false);
    userdoc->setEnabled(false);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a native KDE theme.\n"
    										"Use this template to create native KDE widget themes in C++."));
	}
    										
  else if (qt2normalitem->isSelected() &&strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/qtApp.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(false);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(true);
    progicon->setChecked(true);
    miniicon->setEnabled(true);
    miniicon->setChecked(true);
    miniload->setEnabled(true);
    iconload->setEnabled(true);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a Qt application with a main window containing "
                          "a menubar, toolbar and statusbar, including support for "
                          "a single document-view interface (SDI) model."));
  }
  else if (qt2mdiitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/qtmdi.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(false);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(true);
    progicon->setChecked(true);
    miniicon->setEnabled(true);
    miniicon->setChecked(true);
    miniload->setEnabled(true);
    iconload->setEnabled(true);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a Qt application with a main window containing "
                          "a menubar, toolbar and statusbar, including support for "
                          "a multiple document interface (MDI) model."));
  }
  else if (qextmdiitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/qextmdi.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(false);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(true);
    progicon->setChecked(true);
    miniicon->setEnabled(true);
    miniicon->setChecked(true);
    miniload->setEnabled(true);
    iconload->setEnabled(true);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create an MDI framework based on the QextMDI library and Qt "
                          "Allows to switch between both modes, Toplevel and Childframe. "
                          "Requires QextMDI!!! (www.geocities.com/gigafalk/qextmdi.htm)"));
    KMessageBox::information(0,
                    i18n("If you choose this kind of project:\n\n"
                          "An environment variable QEXTMDIDIR should be set!\n"
                          "Because the wizard searches\n"
                          "the header files in:  $(QEXTMDIDIR)/include\n"
                          "and libqextmdi.so in: $(QEXTMDIDIR)/lib"),
                    i18n("Important hint for a QextMDI project"));
  }
  else if ((citem->isSelected() || cppitem->isSelected())
            && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/terminalApp.png"));
    widget1b->setBackgroundPixmap(pm);
    if (citem->isSelected())
    {
      setTitle(page4, i18n("Headertemplate for .c-files"));
      cppheader->setText( i18n("headertemplate for .c-files") );
    }
    apidoc->setChecked(false);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(false);
    progicon->setChecked(false);
    miniicon->setEnabled(false);
    miniicon->setChecked(false);
    miniload->setEnabled(false);
    iconload->setEnabled(false);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(true);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit")))
    {
      m_finishButton->setEnabled(true);
    }
    apphelp->setText ( (citem->isSelected()) ?
        i18n("Create a C application. The program will run in a terminal "
           "and doesn't contain any support for classes and Graphical User Interface.")
                    :
                    i18n("Create a C++ application. The program will run in a terminal "
           "and doesn't contain any support for a Graphical User Interface."));
  }
  else if (customprojitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/customApp.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(false);
    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(false);
    progicon->setChecked(false);
    miniicon->setEnabled(false);
    miniicon->setChecked(false);
    miniload->setEnabled(false);
    iconload->setEnabled(false);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(false);
    generatesource->setChecked(false);
    generatesource->setEnabled(false);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Creates an empty project to work with existing development projects. "
         "KDevelop will not take care of any Makefiles as those are supposed to "
         "be included with your old project."));
  }
  else if (gnomenormalitem->isSelected() && strcmp (m_cancelButton->text(), i18n("Exit")))
  {
    pm.load(locate("appdata", "pics/gnomeApp.png"));
    widget1b->setBackgroundPixmap(pm);
    apidoc->setChecked(false);
    apidoc->setEnabled(false);

    datalink->setEnabled(false);
    datalink->setChecked(false);
    progicon->setEnabled(false);
    progicon->setChecked(false);
    miniicon->setEnabled(false);
    miniicon->setChecked(false);
    miniload->setEnabled(false);
    iconload->setEnabled(false);
    lsmfile->setChecked(true);
    gnufiles->setChecked(true);
    userdoc->setChecked(false);
    userdoc->setEnabled(false);
    generatesource->setChecked(true);
    generatesource->setEnabled(true);
    if (strcmp(nameline->text(), "") && strcmp (m_cancelButton->text(), i18n("Exit"))) {
       m_finishButton->setEnabled(true);
    }
    apphelp->setText (i18n("Create a GNOME application with session-management, "
         "menubar, toolbar, statusbar and initial event handling."));
  }
  else if (kdeentry->isSelected())
  {
    m_finishButton->setEnabled(false);
    apphelp->setText (i18n("Contains all KDE-compliant\nproject types."));
  }
  else if (qtentry->isSelected())
  {
    m_finishButton->setEnabled(false);
    apphelp->setText (i18n("Contains all Qt-based\nproject types."));
  }
  else if (ccppentry->isSelected())
  {
    m_finishButton->setEnabled(false);
    apphelp->setText (i18n("Contains all C/C++-terminal\nproject types."));
  }
  else if (gnomeentry->isSelected())
  {
    m_finishButton->setEnabled(false);
    apphelp->setText (i18n("Contains all GNOME-compliant\nproject types."));
  }
  else if (othersentry->isSelected())
  {
    m_finishButton->setEnabled(false);
    apphelp->setText (i18n("Contains all individual project types."));
  }
}

// connection of this
void CKAppWizard::slotDefaultClicked() {

  pm.load(locate("appdata", "pics/normalApp.png"));

  setTitle(page4, i18n("Headertemplate for .cpp-files"));

  cppheader->setText( i18n("headertemplate for .cpp-files") );

  widget1b->setBackgroundPixmap(pm);
  applications->setSelected(kde2normalitem,true);
  apidoc->setEnabled(true);
  apidoc->setChecked(true);
  lsmfile->setChecked(true);
  cppheader->setChecked(true);
  hheader->setChecked(true);
  datalink->setChecked(true);
  miniicon->setChecked(true);
  progicon->setChecked(true);
  gnufiles->setChecked(true);
  userdoc->setEnabled(true);
  userdoc->setChecked(true);
  miniload->setEnabled(true);
  iconload->setEnabled(true);
  generatesource->setChecked(true);
  //Benoit Cerrina 17 Dec 99
  KConfig * lSettings = KGlobal::config();
  lSettings->setGroup("General Options");
//  m_dir = lSettings->readEntry ( "ProjectDefaultDir", QDir::homeDirPath()) + QString("/");
  m_dir = lSettings->readEntry ( "ProjectDefaultDir", QDir::homeDirPath());
  if(m_dir.right(1) != "/")
    m_dir+="/";
//  kdDebug() << m_dir << endl;
  directoryline->setText(m_dir);
  //end modif by Benoit Cerrina
  nameline->setText(0);
  m_finishButton->setEnabled(false);
  miniload->setPixmap(locate("appdata", "templates/lo16-app-appicon.png"));
  iconload->setPixmap(locate("appdata", "templates/lo32-app-appicon.png"));

  QFile cppIODev(locate("appdata", "templates/cpp_template"));
  if (cppIODev.open(IO_ReadOnly))
  {
    QTextStream textStream(&cppIODev);
    cppedit->clear();
    cppedit->insertText(&textStream);
  }

  QFile hIODev(locate("appdata", "templates/header_template"));
  if (hIODev.open(IO_ReadOnly))
  {
    QTextStream textStream(&hIODev);
    hedit->clear();
    hedit->insertText(&textStream);
  }

  authorline->setText(m_author_name);
  emailline->setText(m_author_email);
  versionline->setText("0.1");
  messageline->setText ("new project started");
  vendorline->setText(QString(nameline->text()).lower());
  releaseline->setText("start");
  projectlocationline->setText(0);
  vsLocation->setText(QDir::homeDirPath() + "/cvsroot");
  vsBox->setCurrentItem(0);
  modifyVendor= false;
  modifyDirectory= false;
  slotVSBoxChanged(0);
}

// connection of nameline
void CKAppWizard::slotProjectnameEntry(const QString&_projectname)
{
  if(_projectname.isEmpty())
        setNextEnabled( page1, false );
  else
        setNextEnabled( page1, true );
  int position = nameline->cursorPosition();
  nametext = nameline->text();
  nametext = nametext.stripWhiteSpace();
  int length = nametext.length();
  int i = 0;
  QString endname = "";
  QString first = "";
  QString end = nametext;
  QRegExp regexp1 ("[a-zA-Z]");
  QRegExp regexp2 ("[a-zA-Z0-9_]");
  if (!length==0) {
    for (i=0;i<length;i++) {
      first = end.left(1);
      end = end.right(length-i-1);
      if (i==0) {
        if (regexp1.match(first) == -1) {
            first = "";
        }
        else {

          first = first.upper();
        }
      }
      else {
        if (regexp2.match(first) == -1) {
            first = "";
        }
      }
      endname = endname.append(first);
    }
  }

  nameline->setText(endname);
  if (!modifyDirectory) {
     directoryline->setText(m_dir + endname.lower());
  }

  if (!modifyVendor) {
    vendorline->setText(endname.lower());
  }

  if (nametext == "" || gnomeentry->isSelected() || kdeentry->isSelected() || qtentry->isSelected() ||
       ccppentry->isSelected() || othersentry->isSelected()) {
      m_finishButton->setEnabled(false);
  }
  else {
      m_finishButton->setEnabled(true);
  }
  nameline->setCursorPosition(position);
}

// connection of directoryline
void CKAppWizard::slotDirectoryEntry(const QString&)
{
  if(directoryline->hasFocus()) {
    modifyDirectory = true;
  }
  QString directory = directoryline->text();
  QString enddir;
  if (directory.right(1) == "/") {
    directory = directory.left(directory.length() - 1);
  }
  int pos;
  pos = directory.findRev ("/");
  enddir = directory.right (directory.length() - pos -1);

  projectlocationline->setText(enddir);

}

// connection of iconload
void CKAppWizard::slotIconButtonClicked() {
////  QStrList iconlist;
//  KIconDialog iload;
////  iconlist.append (locate("icon", "kdevelop"));
////  iconlist.append (locate("icon","/share/icons"));
//
////  iload.setDir(&iconlist);
////  iload.selectIcon(name1,"*");
//#warning FIXME where do the kdevelop icons reside???
//  name1= iload.selectIcon ();
//  if (!name1.isEmpty() )
//    iconload->setPixmap(KGlobal::iconLoader()->loadIcon(name1, KIcon::SizeLarge));
}

// connection of miniload
void CKAppWizard::slotMiniIconButtonClicked() {
////  QStrList miniiconlist;
//  KIconDialog  mload;
////  miniiconlist.append (locate("icon", "/mini"));
////  miniiconlist.append (locate("","")+"/share/icons/mini");
////  mload.setDir(&miniiconlist);
////  mload.selectIcon(name2,"*");
////  if (!name2.isEmpty() )
////    miniload->setPixmap(KGlobal::iconLoader()->loadMiniIcon(name2));
//#warning FIXME where do the kdevelop icons reside???
//  name2= mload.selectIcon ();
//  if (!name2.isEmpty() )
//    miniload->setPixmap(KGlobal::iconLoader()->loadIcon(name2, KIcon::Small));
}

// activate and deactivate the iconbutton
void CKAppWizard::slotProgIconClicked() {
  if (progicon->isChecked()) {
    iconload->setEnabled(true);
  }
  else {
    iconload->setEnabled(false);
  }
}

// activate and deactivate the miniiconbutton
void CKAppWizard::slotMiniIconClicked() {
  if (miniicon->isChecked()) {
    miniload->setEnabled(true);
  }
  else {
    miniload->setEnabled(false);
  }
}

// activate and deactivate the headerloadbutton, headernewbutton, headeredit
void CKAppWizard::slotHeaderHeaderClicked() {
  if (hheader->isChecked()) {
    hload->setEnabled(true);
    hnew->setEnabled(true);
    hedit->setEnabled(true);
  }
  else {
    hload->setEnabled(false);
    hnew->setEnabled(false);
    hedit->setEnabled(false);
  }
}

// activate and deactivate the cpploadbutton, cppnewbutton, cppedit
void CKAppWizard::slotCppHeaderClicked() {
  if (cppheader->isChecked()) {
    cppload->setEnabled(true);
    cppnew->setEnabled(true);
    cppedit->setEnabled(true);
  }
  else {
    cppload->setEnabled(false);
    cppnew->setEnabled(false);
    cppedit->setEnabled(false);
  }
}

void CKAppWizard::slotProcessExited() {

  QString directory = directoryline->text();
  QString direct = directoryline->text();
  QString prj_str, prj_dir;
  QDir dir;

  // PLEASE
  /*
        DON´T make a 'return' inside this function...
    It´s IMPORTANT to let run "processesend.pl" at the end
    of this function...
    because this removes "entries.XXXXXX", which is a temporary file
  */
  if (vsBox->currentItem() != 0) {
    prj_dir = locateLocal("appdata", "kdeveloptemp") + "/"; // QDir::homeDirPath() + "/.kde/share/apps/kdevelop/kdeveloptemp/";
    prj_str = prj_dir + namelow + ".kdevprj";
  }
  else {
    prj_dir = directory + "/";
    prj_str = prj_dir + namelow + ".kdevprj";
  }

  int pos;
  if (direct.right(1) == "/" && direct.length()>1)
    direct=direct.left(direct.length()-1);
  if ((pos=direct.findRev('/'))>1)
    direct=direct.left(pos);
  dir.setPath(direct);

  project = new CProject(prj_str);
  project->createEmptyProject();
  project->setKDevPrjVersion(KDEVELOP_PRJ_VERSION);

  // Remove sources now... if desired
  if (!generatesource->isChecked())
  {
    removeSources(prj_dir);
  }


  if (cppitem->isSelected()) {
    project->setProjectType("normal_cpp");
  }
  else if (citem->isSelected()) {
    project->setProjectType("normal_c");
  }
  else if (kde2miniitem->isSelected()) {
    project->setProjectType("mini_kde2");
  }
  else if (kde2normalitem->isSelected()) {
    project->setProjectType("normal_kde2");
  }
  else if (kde2mdiitem->isSelected()) {
    project->setProjectType("mdi_kde2");
  }
  else if (qt2normalitem->isSelected()) {
    project->setProjectType("normal_qt2");
  }
  else if (qt2mdiitem->isSelected()) {
    project->setProjectType("mdi_qt2");
  }
  else if (qextmdiitem->isSelected()) {
    project->setProjectType("mdi_qextmdi");
  }
  else if (gnomenormalitem->isSelected()) {
    project->setProjectType("normal_gnome");
  }
  else if (kickeritem->isSelected()){
   project->setProjectType("kicker_app");
  }
  else if (kpartitem->isSelected()){
    project->setProjectType("kpart_plugin");
  }
  else if (kioslaveitem->isSelected()){
    project->setProjectType("kio_slave");
  }
  else if (kthemeitem->isSelected()){
    project->setProjectType("ktheme");
  }
  else if (kcmoduleitem->isSelected()){
    project->setProjectType("kc_module");
  }
//  else if (sharedlibitem->isSelected()){
//    project->setProjectType("shared_lib");
//  }
  else if (customprojitem->isSelected()) {
    project->setProjectType("normal_empty");
  }

  project->setProjectName (nameline->text());
  project->setSubDir (namelow + "/");
  project->setAuthor (authorline->text());
  project->setEmail (emailline->text());
  project->setVersion (versionline->text());
  
  if (userdoc->isChecked()) {
    if(project->isKDE2Project())
      project->setSGMLFile (directory + "/doc/en/index.docbook");
    else
      project->setSGMLFile (directory + "/" + namelow + "/docs/en/index.sgml");
  }
  project->setBinPROGRAM (namelow);
  project->setLDFLAGS (" ");
  project->setMakeOptions ("-j1");
  project->setCXXFLAGS ("-O0 -g3 -Wall");   // default value is to use debugging

  if ( kickeritem->isSelected()) {
   project->setLDADD( " $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)");
  }
  if ( kpartitem->isSelected()) {
   project->setLDADD( " $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KPARTS) $(LIB_KHTML)");
  }
  if ( kioslaveitem->isSelected()) {
   project->setLDADD( " $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KIO)");
  }
  if ( kcmoduleitem->isSelected()) {
   project->setLDADD( " $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) ");
  }
  if ( kthemeitem->isSelected()) {
   project->setLDADD( " $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)");
  }
  if ( kde2miniitem->isSelected()) {
    project->setLDADD (" $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) ");
  }
  else if (kde2normalitem->isSelected() || kde2mdiitem->isSelected()) {
    project->setLDADD (" $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) ");
  }
  else if (qt2normalitem->isSelected() || qt2mdiitem->isSelected()) {
    project->setLDADD (" $(LIB_QPE) $(LIB_QT)");
  }
  else if (qextmdiitem->isSelected()) {
    project->setLDADD (" $(LIB_QT) -lqextmdi");
  }
  else if (gnomenormalitem->isSelected()) {
    project->setLDADD (" $(GNOMEUI_LIBS) $(GNOME_LIBDIR)");
  }

  project->setConfigureArgs(createConfigureArgs());

  QStrList sub_dir_list;
  TMakefileAmInfo makeAmInfo;
  makeAmInfo.rel_name = "Makefile.am";
  makeAmInfo.type = "normal";
  sub_dir_list.append(namelow);
  // Added 'kdenormaloglitem...' by Robert Wheat, 01-22-2000, OpenGL(tm) support
  if (project->isKDE2Project())
  {
    sub_dir_list.append("po");
  }

  if (gnomenormalitem->isSelected()){
    sub_dir_list.append("macros");
    sub_dir_list.append("pixmaps");
  }
  makeAmInfo.sub_dirs = sub_dir_list;
  project->addMakefileAmToProject (makeAmInfo.rel_name,makeAmInfo);

  makeAmInfo.rel_name =  namelow + "/Makefile.am";
  sub_dir_list.clear();
  if(kickeritem->isSelected() || kpartitem->isSelected()||kioslaveitem->isSelected()
  		|| kthemeitem->isSelected()||kcmoduleitem->isSelected()/*|| sharedlibitem->isSelected()*/){
    makeAmInfo.type = "shared_library";
  }
  else {
   makeAmInfo.type = "prog_main";
  }

  makeAmInfo.sub_dirs = sub_dir_list;
  project->addMakefileAmToProject (makeAmInfo.rel_name,makeAmInfo);

  if(project->isKDE2Project())
    makeAmInfo.rel_name = "doc/Makefile.am";
  else
    makeAmInfo.rel_name =  namelow + "/docs/Makefile.am";

  makeAmInfo.type = "normal";
  sub_dir_list.clear();
  makeAmInfo.sub_dirs = sub_dir_list;
  if(!gnomenormalitem->isSelected() && !kthemeitem->isSelected()){
    project->addMakefileAmToProject (makeAmInfo.rel_name,makeAmInfo);
  }

  if(project->isKDE2Project())
    makeAmInfo.rel_name = "doc/en/Makefile.am";
  else
    makeAmInfo.rel_name =  namelow + "/docs/en/Makefile.am";

  makeAmInfo.type = "normal";
  sub_dir_list.clear();
  makeAmInfo.sub_dirs = sub_dir_list;
  if(!gnomenormalitem->isSelected() && !kthemeitem->isSelected()){
    project->addMakefileAmToProject (makeAmInfo.rel_name,makeAmInfo);
  }

  if (project->isKDE2Project() && CToolClass::searchProgram("xgettext"))
  {
    makeAmInfo.rel_name = "po/Makefile.am";
    makeAmInfo.type = "po";
    sub_dir_list.clear();
    makeAmInfo.sub_dirs = sub_dir_list;
    project->addMakefileAmToProject (makeAmInfo.rel_name,makeAmInfo);
  }

  project->setVCSystem(vsBox->currentItem()==0 ? QString("None") : QString("CVS"));

  TFileInfo fileInfo;
  fileInfo.rel_name = namelow + ".kdevprj";
  fileInfo.type = DATA;
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->addFileToProject (namelow + ".kdevprj",fileInfo);

  if(project->isKDE2Project() || project->isQt2Project() ||
     project->getProjectType()=="normal_cpp" || project->getProjectType()=="normal_c"){
  fileInfo.rel_name = "admin";
  fileInfo.type = DATA;
  fileInfo.dist = true;
  fileInfo.install = false;
  fileInfo.install_location = "";
  project->addFileToProject ("admin",fileInfo);
	}

  if (gnufiles->isChecked()) {
    fileInfo.rel_name = "AUTHORS";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("AUTHORS",fileInfo);

    fileInfo.rel_name = "COPYING";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("COPYING",fileInfo);

    fileInfo.rel_name = "ChangeLog";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("ChangeLog",fileInfo);

    fileInfo.rel_name = "INSTALL";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("INSTALL",fileInfo);

    fileInfo.rel_name = "README";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("README",fileInfo);

    fileInfo.rel_name = "TODO";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject ("TODO",fileInfo);

  }

  QDir toplevel(prj_dir, "README.*", 0, QDir::Files | QDir::Readable);
  for (unsigned i=0; i<toplevel.count(); i++)
  {
    fileInfo.rel_name = toplevel[i];
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (toplevel[i], fileInfo);
  }

  QFileInfo news(prj_dir + "/NEWS");
  if (news.exists())
  {
    if (gnufiles->isChecked())
    {
     fileInfo.rel_name = "NEWS";
     fileInfo.type = DATA;
     fileInfo.dist = true;
     fileInfo.install = false;
     fileInfo.install_location = "";
     project->addFileToProject ("NEWS",fileInfo);
    }
    else
      QFile(prj_dir + "/NEWS").remove();
  }

  if (lsmfile->isChecked()) {
    fileInfo.rel_name = namelow + ".lsm";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (namelow + ".lsm",fileInfo);
  }

  if (generatesource->isChecked() &&!kickeritem->isSelected()&&!kpartitem->isSelected()
  	&&!kioslaveitem->isSelected()&&!kthemeitem->isSelected()&&  !kcmoduleitem->isSelected())
  {
    QString extension= (citem->isSelected() || gnomenormalitem->isSelected()) ? "c" : "cpp";
    fileInfo.rel_name = namelow + "/main."+extension;
    fileInfo.type = CPP_SOURCE;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (namelow + "/main."+extension,fileInfo);
  }

  if (gnomenormalitem->isSelected()){
    if (generatesource->isChecked()) {

      fileInfo.rel_name = namelow + "/main.h";
      fileInfo.type = CPP_HEADER;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/main.h",fileInfo);

      fileInfo.rel_name = namelow + "/app.c";
      fileInfo.type = CPP_SOURCE;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/app.c",fileInfo);

      fileInfo.rel_name = namelow + "/app.h";
      fileInfo.type = CPP_HEADER;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/app.h",fileInfo);

      fileInfo.rel_name = namelow + "/menus.c";
      fileInfo.type = CPP_SOURCE;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/menus.c",fileInfo);

      fileInfo.rel_name = namelow + "/menus.h";
      fileInfo.type = CPP_HEADER;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/menus.h",fileInfo);

      fileInfo.rel_name = namelow + ".desktop";
      fileInfo.type = DATA;
      fileInfo.dist = true;
      fileInfo.install = true;
      fileInfo.install_location = "$(datadir)/gnome/apps/Applications/";
      project->addFileToProject (namelow + ".desktop",fileInfo);

      fileInfo.rel_name = "pixmaps/" + namelow +"-logo.png";
      fileInfo.type = DATA;
      fileInfo.dist = true;
      fileInfo.install = true;
      fileInfo.install_location = "$(datadir)/pixmaps";
      project->addFileToProject ("pixmaps/" + namelow +"-logo.png",fileInfo);
    }
  }
  if (!citem->isSelected() && !cppitem->isSelected() && !gnomenormalitem->isSelected()) {
    if (generatesource->isChecked()) {
      fileInfo.rel_name = namelow + "/" + namelow + ".cpp";
      fileInfo.type = CPP_SOURCE;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + ".cpp",fileInfo);

      fileInfo.rel_name = namelow + "/" + namelow + ".h";
      fileInfo.type = CPP_HEADER;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + ".h",fileInfo);
    }
  }

  if (kde2normalitem->isSelected() || kde2mdiitem->isSelected() ||
    project->isQt2Project() ||
    qextmdiitem->isSelected())
  {
     if (generatesource->isChecked()) {
      fileInfo.rel_name = namelow + "/" + namelow + "doc.cpp";
      fileInfo.type = CPP_SOURCE;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + "doc.cpp",fileInfo);

      fileInfo.rel_name = namelow + "/" + namelow + "doc.h";
      fileInfo.type = CPP_HEADER;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + "doc.h",fileInfo);

      fileInfo.rel_name = namelow + "/" + namelow + "view.cpp";
      fileInfo.type = CPP_SOURCE;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + "view.cpp",fileInfo);

      fileInfo.rel_name = namelow + "/" + namelow + "view.h";
      fileInfo.type = CPP_HEADER;
      fileInfo.dist = true;
      fileInfo.install = false;
      fileInfo.install_location = "";
      project->addFileToProject (namelow + "/" + namelow + "view.h",fileInfo);

      if (qextmdiitem->isSelected()){
        fileInfo.rel_name = namelow + "/resource.h";
        fileInfo.type = CPP_HEADER;
        fileInfo.dist = true;
        fileInfo.install = false;
        fileInfo.install_location = "";
        project->addFileToProject (namelow + "/resource.h",fileInfo);
      }

      if(kde2normalitem->isSelected() || kde2mdiitem->isSelected()){
        fileInfo.rel_name = namelow + "/" + namelow + "ui.rc";
        fileInfo.type = DATA;
        fileInfo.dist = true;
        fileInfo.install = false;
        fileInfo.install_location = "";
        project->addFileToProject (namelow + "/" + namelow + "ui.rc",fileInfo);
      }
      if( qextmdiitem->isSelected()) {
        fileInfo.rel_name = namelow + "/" + "tabprocessingeditwidget.cpp";
        fileInfo.type = CPP_SOURCE;
        fileInfo.dist = true;
        fileInfo.install = false;
        fileInfo.install_location = "";
        project->addFileToProject (namelow + "/" + "tabprocessingeditwidget.cpp",fileInfo);

        fileInfo.rel_name = namelow + "/" + "tabprocessingeditwidget.h";
        fileInfo.type = CPP_HEADER;
        fileInfo.dist = true;
        fileInfo.install = false;
        fileInfo.install_location = "";
        project->addFileToProject (namelow + "/" + "tabprocessingeditwidget.h",fileInfo);
      }
    }
  }

  if(kickeritem->isSelected()){
        fileInfo.rel_name = namelow +  "/myview.ui";
        fileInfo.type = CPP_SOURCE;
        fileInfo.dist = true;
        fileInfo.install = false;
        fileInfo.install_location = "";
        project->addFileToProject (namelow +  "/myview.ui",fileInfo);
  }

  if(kpartitem->isSelected()){
        fileInfo.rel_name = namelow + "/" +  namelow + ".rc";
        fileInfo.type = DATA;
        fileInfo.dist = true;
        fileInfo.install = true;
        fileInfo.install_location = " $(kde_datadir)/khtml/kpartplugins/" + namelow + ".rc";
        project->addFileToProject (namelow + "/" +  namelow + ".rc",fileInfo);
  }
  if(kioslaveitem->isSelected()){
        fileInfo.rel_name = namelow + "/" +  namelow + ".protocol";
        fileInfo.type = DATA;
        fileInfo.dist = true;
        fileInfo.install = true;
        fileInfo.install_location = " $(kde_servicesdir)/" + namelow + ".protocol";
        project->addFileToProject (namelow + "/" +  namelow + ".protocol",fileInfo);
  }
  if(kthemeitem->isSelected()){
        fileInfo.rel_name = namelow + "/" +  namelow + ".themerc";
        fileInfo.type = DATA;
        fileInfo.dist = true;
        fileInfo.install = true;
        fileInfo.install_location = " $(kde_datadir)/kstyle/themes/" +  namelow + ".themerc";
        project->addFileToProject (namelow + "/" +  namelow + ".themerc",fileInfo);
  }

  if (datalink->isChecked()) {
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = true;
    if (kickeritem->isSelected()||kde2miniitem->isSelected() || kde2normalitem->isSelected() || kde2mdiitem->isSelected()) {
      fileInfo.rel_name = namelow + "/" + namelow + ".desktop";
      fileInfo.install_location =
        (kickeritem->isSelected() ? "$(kde_datadir)/kicker/applets/" :
	 "$(kde_appsdir)/Applications/") + namelow + ".desktop";
      project->addFileToProject (namelow + "/" + namelow + ".desktop",fileInfo);
     }
     else if(kcmoduleitem->isSelected()){
      fileInfo.rel_name = namelow + "/" + namelow + ".desktop";
      fileInfo.install_location = "$(kde_appsdir)/Settings/" + namelow + ".desktop";
      project->addFileToProject (namelow + "/" + namelow + ".desktop",fileInfo);
     }
    else{
      fileInfo.rel_name = namelow + "/" + namelow + ".kdelnk";
      fileInfo.install_location = "$(kde_appsdir)/Applications/" + namelow + ".kdelnk";
      project->addFileToProject (namelow + "/" + namelow + ".kdelnk",fileInfo);
    }
  }

  if (progicon->isChecked()) {
    fileInfo.rel_name = namelow + "/lo32-app-" + namelow + ".png";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!(qt2normalitem->isSelected()|| qt2mdiitem->isSelected() || qextmdiitem->isSelected())) {
      fileInfo.install = true;
      if (kcmoduleitem->isSelected()|| kpartitem->isSelected()||
      	kickeritem->isSelected()||kde2miniitem->isSelected() ||
		kde2normalitem->isSelected() || kde2mdiitem->isSelected())
			fileInfo.install_location = "$(kde_icondir)/locolor/32x32/apps/" +namelow + ".png";
		else
    	   fileInfo.install_location = "$(kde_icondir)/" + namelow + ".png";
    }
    else {
      fileInfo.install = false;
      fileInfo.install_location = "";
    }
    project->addFileToProject (namelow + "/lo32-app-" + namelow + ".png",fileInfo);
  }

  if (miniicon->isChecked()) {
    fileInfo.rel_name = namelow + "/lo16-app-" + namelow + ".png";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    if (!( qt2normalitem->isSelected()|| qt2mdiitem->isSelected() || qextmdiitem->isSelected())) {
      fileInfo.install = true;
      if (kpartitem->isSelected()||kickeritem->isSelected()||kde2miniitem->isSelected() || kde2normalitem->isSelected() || kde2mdiitem->isSelected())
        fileInfo.install_location = "$(kde_icondir)/locolor/16x16/apps/" + namelow + ".png";
      else
        fileInfo.install_location = "$(kde_minidir)/" + namelow + ".png";
    }
    else {
      fileInfo.install = false;
      fileInfo.install_location = "";
    }
    project->addFileToProject (namelow + "/lo16-app-" + namelow + ".png",fileInfo);
  }

  if ( qt2normalitem->isSelected()|| qt2mdiitem->isSelected() || qextmdiitem->isSelected()) {
    fileInfo.rel_name = namelow + "/filenew.xpm";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (namelow + "/filenew.xpm",fileInfo);

    fileInfo.rel_name = namelow + "/filesave.xpm";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (namelow + "/filesave.xpm",fileInfo);

    fileInfo.rel_name = namelow + "/fileopen.xpm";
    fileInfo.type = DATA;
    fileInfo.dist = true;
    fileInfo.install = false;
    fileInfo.install_location = "";
    project->addFileToProject (namelow + "/fileopen.xpm",fileInfo);
  }

  if (userdoc->isChecked() && !project->isKDE2Project() )
  {
    for (int i=0; i<7; i++){
      QString num;
      num.setNum(i);
      num.prepend("-");
      if(i==0) num="";
      fileInfo.rel_name = namelow + "/docs/en/index"+num+".html";
      fileInfo.type = DATA;
      fileInfo.dist = true;
      if (!cppitem->isSelected() && !citem->isSelected()) {
        fileInfo.install = true;
        if ( project->isQt2Project() || qextmdiitem->isSelected())
          fileInfo.install_location = "$(prefix)/doc/";
        else
          fileInfo.install_location = "$(kde_htmldir)/en/";
        fileInfo.install_location += namelow+ "/index"+num+".html";
      }
      project->addFileToProject (namelow + "/docs/en/index"+num+".html",fileInfo);
    }
  }
  if (userdoc->isChecked() && project->isKDE2Project() && !kthemeitem->isSelected())
  {
      fileInfo.rel_name ="doc/en/index.docbook";
      fileInfo.type = DATA;
      fileInfo.dist = true;
      fileInfo.install = false;
      project->addFileToProject ("doc/en/index.docbook",fileInfo);
  }
  QStringList group_filters;
  group_filters.append("*");
  project->addLFVGroup (i18n("Others"),"");
  project->setFilters(i18n("Others"),group_filters);



  if (gnufiles->isChecked()) {
    group_filters.clear();
    group_filters.append("AUTHORS");
    group_filters.append("COPYING");
    group_filters.append("ChangeLog");
    group_filters.append("INSTALL");
    group_filters.append("README");
    group_filters.append("TODO");
    group_filters.append("NEWS");
    project->addLFVGroup ("GNU","");
    project->setFilters("GNU",group_filters);
  }

  if (project->isKDE2Project() || project->isKDEProject())
  {
    group_filters.clear();
    group_filters.append("*.po");
    project->addLFVGroup (i18n("Translations"),"");
    project->setFilters(i18n("Translations"),group_filters);
  }
  if (project->isQt2Project())
  {
    group_filters.clear();
    group_filters.append("*.ts");
    project->addLFVGroup (i18n("Translations"),"");
    project->setFilters(i18n("Translations"),group_filters);
  }

  if (!cppitem->isSelected() && !citem->isSelected() &&!gnomenormalitem->isSelected()) {
    group_filters.clear();
    group_filters.append("*.kdevdlg");
    group_filters.append("*.ui");
    group_filters.append("*.rc");
    group_filters.append("*.dlg");
    project->addLFVGroup (i18n("User Interface"),"");
    project->setFilters(i18n("User Interface"),group_filters);
  }


  group_filters.clear();
  group_filters.append("*.cpp");
  group_filters.append("*.c");
  group_filters.append("*.cc");
  group_filters.append("*.C");
  group_filters.append("*.cxx");
  group_filters.append("*.ec");
  group_filters.append("*.ecpp");
  group_filters.append("*.lxx");
  group_filters.append("*.l++");
  group_filters.append("*.ll");
  group_filters.append("*.l");
  project->addLFVGroup (i18n("Sources"),"");
  project->setFilters(i18n("Sources"),group_filters);

  group_filters.clear();
  group_filters.append("*.h");
  group_filters.append("*.hh");
  group_filters.append("*.hxx");
  group_filters.append("*.hpp");
  group_filters.append("*.H");
  project->addLFVGroup (i18n("Headers"),"");
  project->setFilters(i18n("Headers"),group_filters);

  project->writeProject ();
  project->updateMakefilesAm ();

  disconnect(q,SIGNAL(processExited(KProcess *)),this,SLOT(slotProcessExited()));
  connect(q,SIGNAL(processExited(KProcess *)),this,SLOT(slotMakeEnd()));

  KShellProcess p("/bin/sh");
  if (vsBox->currentItem() == 1)
  {
    dir.setCurrent(locateLocal("appdata", "kdeveloptemp"));   //QDir::homeDirPath() + "/.kde/share/apps/kdevelop/kdeveloptemp");

    QString message=messageline->text();
    if (!message.isEmpty())
      message.replace(QRegExp("'"), "'\"'\"'");
    QString import = (QString) "cvs -d " + vsLocation->text() + (QString) " import -m \'" + message +
       (QString) "\' " + projectlocationline->text() + (QString) " " + vendorline->text() +
       (QString) " " + releaseline->text();
    p << import;
    p.start(KProcess::Block, KProcess::AllOutput);
  }

  if (vsBox->currentItem() != 0)
  {
    // clear projectfile before removing temp project generation
    project->writeProject();
    delete project;
    project = 0;

    dir.setCurrent(QDir::homeDirPath());
    QString deltemp = "rm -r -f " + locateLocal("appdata", "kdeveloptemp");   //QDir::homeDirPath() + "/.kde/share/apps/kdevelop/kdeveloptemp";
    p.clearArguments();
    p << deltemp;
    p.start(KProcess::Block, KProcess::AllOutput);

    dir.setCurrent(direct);
    QString checkout = "cvs -d " + (QString) vsLocation->text() + " co " +
                        (QString) projectlocationline->text();
    p.clearArguments();
    p << checkout;
    p.start(KProcess::Block, KProcess::AllOutput);

    // reset now projectfile to actual directory
    project=new CProject(QString(projectlocationline->text()) + "/" + namelow + ".kdevprj");
		if (project->prepareToReadProject()) {
      project->readProject();
    }
  }


  q->clearArguments();
  *q << "perl" << locate("appdata", "tools/processesend.pl");
  if (!entriesfname.isEmpty())
    *q << entriesfname;

  *q << locateLocal("appdata", "");

  q->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}

// enable m_cancelbutton if everything is done
void CKAppWizard::slotMakeEnd() {
  nametext = nameline->text();
  nametext = nametext.lower();
  directorytext = directoryline->text();
  QString cvscommand;

  if (project->isValid()) {
    project->writeProject ();
    project->updateMakefilesAm ();
  }
  QString message=messageline->text();
  if (!message.isEmpty())
   message.replace(QRegExp("'"), "'\"'\"'");

  if (vsBox->currentItem() == 1 && !cvscommand.isEmpty())
  {
   KShellProcess p("/bin/sh");
   cvscommand += (QString) "cvs commit -m \'" + message + "\' .";
   p.clearArguments();
   p << cvscommand;
   p.start(KProcess::Block, KProcess::AllOutput);
  }

  QFileInfo pot (directorytext + "/po/" + nametext +".pot");
  if (vsBox->currentItem() == 1 && pot.exists())
  {
   KShellProcess p("/bin/sh");
   cvscommand="cd "+ directorytext + "/po && cvs add "+ nametext +".pot && ";
   cvscommand += QString("cvs commit -m \'") + message + "\' .";
   p.clearArguments();
   p << cvscommand;
   p.start(KProcess::Block, KProcess::AllOutput);
  }

  m_cancelButton->setEnabled(true);
  gen_prj = true;
}

// return the directory with the projectfile
QString CKAppWizard::getProjectFile() {
  nametext = nameline->text();
  nametext = nametext.lower();
  directorytext = directoryline->text();
  if(QString(directoryline->text()).right(1) == "/"){
    directorytext = directorytext + nametext + ".kdevprj";
  }
  else{
    directorytext = directorytext + "/" + nametext + ".kdevprj";
  }
  delete (directoryline);
  delete (nameline);
  delete (page0);
  delete (page1);
  delete (page2);
  delete (page3);
  delete (page4);
  delete (page5);
  return directorytext;
}

// return TRUE if a project is generated
bool CKAppWizard::generatedProject(){
  return gen_prj;
}

void CKAppWizard::slotLocationButtonClicked()
{
  QString dirPath = KFileDialog::getExistingDirectory(m_dir);
  m_dir = dirPath + "cvsroot";
  vsLocation->setText(m_dir);
}


void CKAppWizard::slotVSBoxChanged(int item)
{
  bool enable = (item != 0);
  messageline->setEnabled(enable);
  logMessage->setEnabled(enable);
  vendorline->setEnabled(enable);
  vendorTag->setEnabled(enable);
  releaseline->setEnabled(enable);
  releaseTag->setEnabled(enable);
  vsInstall->setEnabled(enable);
  projectVSLocation->setEnabled(enable);
  vsLocation->setEnabled(enable);
  locationbutton->setEnabled(enable);
  qtarch_ButtonGroup_1->setEnabled(enable);
}

void CKAppWizard::slotVendorEntry(const QString&)
{
  if(vendorline->hasFocus()) {
    modifyVendor = true;
  }
}
#include "ckappwizard.moc"
