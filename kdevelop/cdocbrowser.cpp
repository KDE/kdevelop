/***************************************************************************
           cdocbrowser.cpp - 
                             -------------------                                         

    begin                : 20 Jul 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
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


#include "cdocbrowser.h"

#include <kapp.h>
#include <kconfig.h>
#include <kcolorbtn.h>
#include <kcursor.h>
#include <kfontcombo.h>
#include <kfontdialog.h>
#include <kglobal.h>
#include <khtml_settings.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <krun.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <khtml_settings.h>

#include "resource.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qfontdatabase.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qradiobutton.h>
#include <qgrid.h>
#include <qlayout.h>

#include "cfinddoctextdlg.h"

#include <X11/Xlib.h>
#undef Unsorted

//#include <iostream>
//using namespace std;

int  CDocBrowser::fSize = 12;
QFont CDocBrowser::standardFont;
QFont CDocBrowser::fixedFont;
//FB QColor CDocBrowser::bgColor;
QColor CDocBrowser::textColor;
QColor CDocBrowser::linkColor;
QColor CDocBrowser::vLinkColor;
bool CDocBrowser::underlineLinks;
//FB bool CDocBrowser::forceDefaults;

CDocBrowser::CDocBrowser(QWidget*parent,const char* name) :
  KHTMLPart(parent, name)
{
  kdDebug() << "Creating CDocBrowser !" << endl;
  setDocBrowserOptions();      //  setup the options

  doc_pop = new QPopupMenu();
  doc_pop->insertItem(SmallIconSet("back"),i18n("Back"),this, SLOT(slotURLBack()),0,ID_HELP_BACK);
  doc_pop->insertItem(SmallIconSet("forward"),i18n("Forward"),this,SLOT(slotURLForward()),0,ID_HELP_FORWARD);
  doc_pop->insertSeparator();
  doc_pop->insertItem(SmallIconSet("editcopy"),i18n("Copy"),this, SLOT(slotCopyText()),0,ID_EDIT_COPY);
  doc_pop->insertItem(SmallIconSet("bookmark_add"),i18n("Toggle Bookmark"),this, SIGNAL(signalBookmarkToggle()),0,ID_BOOKMARKS_TOGGLE);
  doc_pop->insertItem(i18n("View in new window"), this, SLOT(slotViewInKFM()),0,ID_VIEW_IN_KFM);
  doc_pop->insertSeparator();
  doc_pop->insertItem(SmallIconSet("grep"),i18n("grep: "), this, SLOT(slotGrepText()), 0, ID_EDIT_SEARCH_IN_FILES);
  doc_pop->insertItem(SmallIconSet("help"),i18n("look up: "),this, SLOT(slotSearchText()),0,ID_HELP_SEARCH_TEXT);
  doc_pop->insertItem(SmallIconSet("help"),i18n("manpage: "),this, SLOT(slotManpage()),0,ID_HELP_MANPAGE);

//  view()->setFocusPolicy( QWidget::StrongFocus );
  connect(this, SIGNAL( popupMenu( const QString&, const QPoint & ) ),
          this, SLOT( slotPopupMenu( const QString&, const QPoint & ) ) );
  connect(this, SIGNAL( setWindowCaption ( const QString&) ), this, SLOT( slotSetFileTitle( const QString&) ) );

//XXX  KConfig *config = KGlobal::config();
//XXX  KHTMLSettings* htmlsettings = settings();
//XXX  htmlsettings->init(config); // no reset, just additionally

  kdDebug() << "End CDocBrowser creation !" << endl;
}

CDocBrowser::~CDocBrowser(){

  kdDebug() << "deleting CDocBrowser !" << endl;

  delete doc_pop;
  doc_pop=0l;
}


void CDocBrowser::slotViewInKFM()
{
  kdDebug() << "CDocBrowser::slotViewInKFM !" << endl;
  new KRun(currentURL());
}

void CDocBrowser::showURL(const QString& url, bool reload)
{
  if (url.isEmpty())
    return;

  complete_url=url;

  // in some cases KHTMLView return "file:/file:/...." (which might be a bug in kdoc?)
  // Anyway clean up the url from this error
  if (complete_url.left(12)=="file:/file:/")
    complete_url=complete_url.mid(6, complete_url.length());

  QString url_wo_ref=complete_url; // without ref

  int pos = complete_url.findRev('#');
  int len = complete_url.length();

  QString ref;
  if (pos!=-1)
    ref = complete_url.right(len - pos - 1);

  m_refTitle = ref;

  if (pos!=-1)
    url_wo_ref = complete_url.left(pos);

  if(complete_url.left(7) == "http://" || url_wo_ref.right(4).find("htm", FALSE)==-1)
  {
    if (complete_url.left(5) != "man:/")
    {
      new KRun(complete_url);
      return;
    }
  }
  // workaround for kdoc2 malformed urls in crossreferences to Qt-documentation
  if(complete_url.contains("file%253A/"))
    complete_url.replace( QRegExp("file%253A/"), "" );
    
  if(complete_url.contains("file%3A/"))
    complete_url.replace( QRegExp("file%3A/"), "" );

  if (!reload && (complete_url == old_url)) {
    return;
  }

  kapp->setOverrideCursor( Qt::waitCursor );
  KURL kurl(complete_url);
  openURL(kurl);
  if (ref.isEmpty())
    view()->setContentsPos(0,0);
  kapp->restoreOverrideCursor();

  old_url = complete_url;
  emit completed();  // simulate documentDone to put it in history...
}

void CDocBrowser::setDocBrowserOptions()
{
  kdDebug() << "CDocBrowser::setDocBrowserOptions called !" << endl;
  KConfig *config = KGlobal::config();
  config->setGroup( "HTML Settings" );          // make it consistent between KDE2 and KDE3

  const_cast<KHTMLSettings*>(settings())->init( config, true );
  setURLCursor( KCursor::handCursor() );
}

void CDocBrowser::slotDocBrowserOptions( void )
{
  setDocBrowserOptions();

  if ( !currentURL().isEmpty() )
    showURL(currentURL(), true);
}

void CDocBrowser::slotPopupMenu( const QString&/*url*/, const QPoint & pnt){

  kdDebug() << "CDocBrowser::slotPopupMenu !" << endl;

  QString text;
  int pos;
  if (hasSelection())
  {
    text = selectedText();
    text.replace(QRegExp("^\n"), "");
    pos=text.find("\n");
    if (pos>-1)
     text=text.left(pos);
  }

  if (!text.isEmpty())
  {
    doc_pop->setItemEnabled(ID_EDIT_COPY,true);
    doc_pop->setItemEnabled(ID_HELP_MANPAGE,true);
    doc_pop->setItemEnabled(ID_HELP_SEARCH_TEXT,true);
    doc_pop->setItemEnabled(ID_EDIT_SEARCH_IN_FILES,true);

    if(text.length() > 20 ){
      text = text.left(20) + "...";
    }
    doc_pop->changeItem(SmallIconSet("grep"),i18n("grep: ")+text, ID_EDIT_SEARCH_IN_FILES);
    doc_pop->changeItem(SmallIconSet("help"),i18n("look up: ")+ text,ID_HELP_SEARCH_TEXT);
    doc_pop->changeItem(SmallIconSet("help"),i18n("manpage: ")+ text,ID_HELP_MANPAGE);
  }
  else
  {
    doc_pop->setItemEnabled(ID_EDIT_COPY,false);
    doc_pop->setItemEnabled(ID_HELP_MANPAGE,false);
    doc_pop->setItemEnabled(ID_HELP_SEARCH_TEXT,false);
    doc_pop->setItemEnabled(ID_EDIT_SEARCH_IN_FILES,false);
    doc_pop->changeItem(SmallIconSet("grep"),i18n("grep: "), ID_EDIT_SEARCH_IN_FILES);
    doc_pop->changeItem(SmallIconSet("help"),i18n("look up: "),ID_HELP_SEARCH_TEXT);
    doc_pop->changeItem(SmallIconSet("help"),i18n("manpage: "),ID_HELP_MANPAGE);
  }
  doc_pop->popup(pnt);
}

void CDocBrowser::slotCopyText()
{
  kdDebug() << "CDocBrowser::slotCopyText !" << endl;

  QString text = selectedText();
  if (!text.isEmpty())
  {
    QClipboard *cb = kapp->clipboard();
    cb->setText( text );
  }
}

void CDocBrowser::slotFindTextNext(QString str){

  kdDebug() << "CDocBrowser::slotFindTextNext !" << endl;
#if (QT_VERSION < 300)
  findTextNext(QRegExp(str),true);
#else
  findTextNext(str,true,false,true);
#endif
}

void CDocBrowser::slotSearchText(){

  kdDebug() << "CDocBrowser::slotSearchText !" << endl;
  emit signalSearchText();
}

void CDocBrowser::slotManpage()
{
  kdDebug() << "CDocBrowser::slotManpage" << endl;
  QString text = "man:/"+selectedText();    // +"(3)";
  emit signalManpage(text);
}

void CDocBrowser::slotGrepText(){

  kdDebug() << "CDocBrowser::slotGrepText !" << endl;
  QString text = selectedText();
  emit signalGrepText(text);
}

void CDocBrowser::slotURLBack(){

  kdDebug() << "CDocBrowser::slotURLBack !" << endl;
  emit signalURLBack();
}

void CDocBrowser::slotURLForward(){

  kdDebug() << "CDocBrowser::slotURLForward !" << endl;
  emit signalURLForward();
}

void CDocBrowser::slotSetFileTitle( const QString& title ){

  kdDebug() << "CDocBrowser::slotSetFileTitle :" << endl;
  kdDebug() << "view : " << view() << " title : " << title.data() << endl;
  m_title= title;
}

QString CDocBrowser::currentTitle(){
  return (m_refTitle.isEmpty()) ? m_title : m_refTitle+" - "+m_title;  
}

#if (QT_VERSION < 300)
void  CDocBrowser::urlSelected ( const QString &url, int button, int state, const QString &_target)
{
  KHTMLPart::urlSelected (url, button, state,_target);

  KURL cURL = completeURL( url );
  showURL( cURL.url() ) ;
}
#else
void  CDocBrowser::urlSelected ( const QString &url, int button, int state, const QString &_target, KParts::URLArgs args)
{
  KHTMLPart::urlSelected (url, button, state,_target, args);
  showURL( KURL(baseURL(),url).url() );
}
#endif

void  CDocBrowser::doSearchDialog()
{
  kdDebug() << "CDocBrowser::doSearchDialog" << endl;

  CFindDocTextDlg help_srch_dlg(view(),"Search_for_Help_on");
  connect(&help_srch_dlg, SIGNAL(signalFind(QString)),
	  this, SLOT(slotFindTextNext(QString)));
  help_srch_dlg.exec();
}

//-----------------------------------------------------------------------------

CDocBrowserFont::CDocBrowserFont( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  readOptions();

  QRadioButton *rb;
  QLabel *label;
  QGridLayout *grid1 = new QGridLayout(this,3,2,15,7);
  QButtonGroup *bg = new QButtonGroup( i18n("Font Size"), this );
  bg->setExclusive( TRUE );
  grid1->addMultiCellWidget(bg,0,0,0,1);

  QGridLayout *grid2 = new QGridLayout(bg,1,3,7,7);
  rb = new QRadioButton( i18n("Small"), bg );
  grid2->addWidget(rb,0,0);
  rb->setChecked( fSize == 8 );

  rb = new QRadioButton( i18n("Medium"), bg );
  grid2->addWidget(rb,0,1);
  rb->setChecked( fSize == 10);

  rb = new QRadioButton( i18n("Large"), bg );
  grid2->addWidget(rb,0,2);
  rb->setChecked( fSize == 12);

  label = new QLabel( i18n("Standard Font"), this );
  grid1->addWidget(label,1,0);

  getFontList( standardFonts, false );
  KFontCombo *kfc = new KFontCombo( standardFonts, this, "standardfont_combo" );
  grid1->addWidget(kfc,1,1);

  int i=0;
  for ( QStringList::Iterator it = standardFonts.begin(); it != standardFonts.end(); ++it, i++ )
  {
    if (stdName == *it)
    {
      kfc->setCurrentItem( i );
      break;
    }
  }
  connect( kfc, SIGNAL( activated( const QString& ) ),
            SLOT( slotStandardFont( const QString& ) ) );

  label = new QLabel( i18n( "Fixed Font"), this );
  grid1->addWidget(label,2,0);

  getFontList( fixedFonts, true );
  kfc = new KFontCombo( fixedFonts, this, "fixedwithfont_combo" );
  grid1->addWidget(kfc,2,1);

  i=0;
  for ( QStringList::Iterator it = fixedFonts.begin(); it != fixedFonts.end(); ++it, i++ )
  {
    if ( fixedName == *it )
    {
      kfc->setCurrentItem( i );
      break;
    }
  }
  connect( kfc, SIGNAL( activated( const QString& ) ),
    SLOT( slotFixedFont( const QString& ) ) );

  connect( bg, SIGNAL( clicked( int ) ), SLOT( slotFontSize( int ) ) );
}


void CDocBrowserFont::readOptions()
{
  KConfig *config = KGlobal::config();
  config->setGroup( "HTML Settings" );

  fSize= config->readNumEntry( "MediumFontSize",10 );
  stdName = config->readEntry( "StandardFont" );
  if ( stdName.isEmpty() )
    stdName = KGlobalSettings::generalFont().family();

  fixedName = config->readEntry( "FixedFont" );
  if ( fixedName.isEmpty() )
    fixedName =  KGlobalSettings::fixedFont().family();
}

void CDocBrowserFont::getFontList( QStringList &list, const char *pattern )
{
  int num;

  char **xFonts = XListFonts( qt_xdisplay(), pattern, 2000, &num );

  for ( int i = 0; i < num; i++ )
  {
    addFont( list, xFonts[i] );
  }

  XFreeFontNames( xFonts );
}

void CDocBrowserFont::getFontList( QStringList &list, bool fixed )
{
	// taken from KFontComboBox of KDE-2.2.2
  QFontDatabase dbase;
  QStringList lstSys(dbase.families( false ));

  // Since QFontDatabase doesn't have any easy way of returning just
  // the fixed width fonts, we'll do it in a very hacky way
  if (fixed)
  {
    QStringList lstFixed;
    for (QStringList::Iterator it = lstSys.begin(); it != lstSys.end(); ++it)
    {
        // To get the fixed with info (known as fixed pitch in Qt), we
        // need to get a QFont or QFontInfo object.  To do this, we
        // need a family name, style, and point size.
        QStringList styles(dbase.styles(*it));
        QStringList::Iterator astyle = styles.begin();

        QFontInfo info(dbase.font(*it, *astyle, 10));
        if (info.fixedPitch())
          lstFixed.append(*it);
    }

    // Fallback.. if there are no fixed fonts found, it's probably a
    // bug in the font server or Qt.  In this case, just use 'fixed'
    if (lstFixed.count() == 0)
      lstFixed.append("fixed");

    lstSys = lstFixed;
  }

  lstSys.sort();

  list = lstSys;
}

void CDocBrowserFont::addFont( QStringList &list, const char *xfont )
{
  const char *ptr = strchr( xfont, '-' );
  if ( !ptr )
    return;
  
  ptr = strchr( ptr + 1, '-' );
  if ( !ptr )
    return;

  QString font = ptr + 1;

  int pos;
  if ( ( pos = font.find( '-' ) ) > 0 )
  {
    font.truncate( pos );

    if ( font.find( "open look", 0, false ) >= 0 )
      return;

    
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
        if ( *it == font )
          return;
    }

    list.append( font );
  }
}

void CDocBrowserFont::slotApplyPressed()
{
  kdDebug() << "CDocBrowserFont::slotApplyPressed !" << endl;

  KConfig *config = KGlobal::config();
  config->setGroup( "HTML Settings" );

  config->writeEntry( "MediumFontSize", fSize );
  config->writeEntry( "MinimumFontSize", 8 );
  config->writeEntry( "StandardFont", stdName );
  config->writeEntry( "FixedFont", fixedName );

  config->sync();
  emit configChanged();
}

void CDocBrowserFont::slotFontSize( int i )
{
  kdDebug() << "CDocBrowserFont::slotFontSize !" << endl;

  if(i==0)
    fSize=8;
  else if(i==1)
    fSize=10;
  else
    fSize=12;
}

void CDocBrowserFont::slotStandardFont( const QString& n )
{
  kdDebug() << "CDocBrowserFont::slotStandardFont !" << endl;
  stdName = n;
}

void CDocBrowserFont::slotFixedFont( const QString& n )
{
  kdDebug() << "CDocBrowserFont::slotFixedFont !" << endl;
  fixedName = n;
}

//-----------------------------------------------------------------------------

CDocBrowserColor::CDocBrowserColor( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
  readOptions();
  QGridLayout *grid1 = new QGridLayout(this,2,6,15,7);

  KColorButton *colorBtn;
  QLabel *label;

//FB  label = new QLabel( i18n("Background Color:"), this );
//FB  grid1->addWidget(label,0,0);

//FB  colorBtn = new KColorButton( bgColor, this );
//FB  grid1->addWidget(colorBtn,0,1);

//FB  connect( colorBtn, SIGNAL( changed( const QColor & ) ),
//FB    SLOT( slotBgColorChanged( const QColor & ) ) );

  label = new QLabel( i18n("Normal text color:"), this );
  grid1->addWidget(label,1,0);

  colorBtn = new KColorButton( textColor, this );
  grid1->addWidget(colorBtn,1,1);
  connect( colorBtn, SIGNAL( changed( const QColor & ) ),
    SLOT( slotTextColorChanged( const QColor & ) ) );

  label = new QLabel( i18n("URL link color:"), this );
  grid1->addWidget(label,2,0);

  colorBtn = new KColorButton( linkColor, this );
  grid1->addWidget(colorBtn,2,1);

  connect( colorBtn, SIGNAL( changed( const QColor & ) ),
    SLOT( slotLinkColorChanged( const QColor & ) ) );

  label = new QLabel( i18n("Followed link color:"), this );
  grid1->addWidget(label,3,0);

  colorBtn = new KColorButton( vLinkColor, this );
  grid1->addWidget(colorBtn,3,1);
  connect( colorBtn, SIGNAL( changed( const QColor & ) ),
    SLOT( slotVLinkColorChanged( const QColor & ) ) );

  QCheckBox *underlineBox = new QCheckBox( i18n("Underline links"), this);
  grid1->addWidget(underlineBox,4,0);
  underlineBox->setChecked(underlineLinks);
  connect( underlineBox, SIGNAL( toggled( bool ) ),
    SLOT( slotUnderlineLinksChanged( bool ) ) );

//FB  QCheckBox *forceDefaultBox = new QCheckBox( i18n("Always use my colors"), this);
//FB  grid1->addWidget(forceDefaultBox,5,0);
//FB  forceDefaultBox->setChecked(forceDefault);
//FB  connect( forceDefaultBox, SIGNAL( toggled( bool ) ),
//FB    SLOT( slotForceDefaultChanged( bool ) ) );
}

void CDocBrowserColor::readOptions()
{
  KConfig *config = KGlobal::config();

//  Colors must be in group [General]
  config->setGroup( "General" ); // conform to KHMTL_Settings

//FB  bgColor = config->readColorEntry( "BgColor", &white );
  textColor = config->readColorEntry( "TextColor", &black );
  linkColor = config->readColorEntry( "linkColor", &blue );
  vLinkColor = config->readColorEntry( "visitedLinkColor", &magenta );
  config->setGroup( "HTML Settings" ); // conform to KHMTL_Settings
  underlineLinks = config->readBoolEntry( "UnderlineLinks", TRUE );
//FB  forceDefault = config->readBoolEntry( "ForceDefaultColors", true );

  changed = false;
}

void CDocBrowserColor::slotApplyPressed()
{
  KConfig *config = KGlobal::config();
//  Colors must be in group [General]
  config->setGroup( "General" );
//FB  config->writeEntry( "BgColor", bgColor );
  config->writeEntry( "TextColor", textColor );
  config->writeEntry( "foreground", textColor );   // KDE 2/3 uses this one
  config->writeEntry( "linkColor", linkColor );
  config->writeEntry( "visitedLinkColor", vLinkColor );
  config->setGroup( "HTML Settings" );
  config->writeEntry( "UnderlineLinks", underlineLinks );
//FB  config->writeEntry( "ForceDefaultColors", forceDefault );

  config->sync();
  emit configChanged();
}

void CDocBrowserColor::slotBgColorChanged( const QColor &col )
{
//FB  if ( bgColor != col )
//FB          changed = true;
//FB  bgColor = col;
}

void CDocBrowserColor::slotTextColorChanged( const QColor &col )
{
  if ( textColor != col )
      changed = true;
  textColor = col;
}

void CDocBrowserColor::slotLinkColorChanged( const QColor &col )
{
  if ( linkColor != col )
          changed = true;
  linkColor = col;
}

void CDocBrowserColor::slotVLinkColorChanged( const QColor &col )
{
  if ( vLinkColor != col )
          changed = true;
  vLinkColor = col;
}

void CDocBrowserColor::slotUnderlineLinksChanged( bool ulinks )
{
  if ( underlineLinks != ulinks )
          changed = true;
  underlineLinks = ulinks;
}

void CDocBrowserColor::slotForceDefaultChanged( bool force )
{
//FB  if ( forceDefault != force )
//FB          changed = true;
//FB  forceDefault = force;
}

//-----------------------------------------------------------------------------


CDocBrowserOptionsDlg::CDocBrowserOptionsDlg( QWidget *parent, const char *name )
  : QTabDialog( parent, name,TRUE ){
  setCaption( i18n("Documentation Browser Options") );

  // resize( 350, 270 );

        setOKButton( i18n("OK") );
        setCancelButton( i18n("Cancel") );
        setApplyButton( i18n("Apply") );

  fontOptions = new CDocBrowserFont( this, i18n("Fonts") );
  addTab( fontOptions, i18n("Fonts") );
  connect( this, SIGNAL( applyButtonPressed() ),
    fontOptions, SLOT( slotApplyPressed() ) );

//  colorOptions = NULL;
//  to use the colors the KHTML settings must be written to group [General]
//  and this will be read in setDocBrowserOptions. Volunteers to fixt it?
  colorOptions = new CDocBrowserColor( this, i18n("Colors") );
  addTab( colorOptions, i18n("Colors") );
  connect( this, SIGNAL( applyButtonPressed() ),
    colorOptions, SLOT( slotApplyPressed() ) );
}


#include "cdocbrowser.moc"
