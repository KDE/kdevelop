/*
 *  File : snippet_widget.cpp
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <qlayout.h>
#include <kpushbutton.h>
#include <klistview.h>
#include <qheader.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <kmessagebox.h>
#include <qsplitter.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/document.h>
#include <kconfig.h>
#include <qtooltip.h>
#include <kpopupmenu.h>
#include <qregexp.h>
#include <qinputdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <qdragobject.h>
#include <qtimer.h>

#include <kdevcore.h>
#include "kdevcore.h"
#include <kdevproject.h>
#include <kdevpartcontroller.h>

#include <snippetdlg.h>
#include <snippetitem.h>

#include "snippet_part.h"
#include "snippet_widget.h"

SnippetWidget::SnippetWidget(SnippetPart *part)
		: KListView(0, "snippet widget"), QToolTip( viewport() ),
		m_part( part )
{

    m_dialog = new SnippetDlg(this, "SnippetDlg", true);

    // init the QPtrList
    _list.setAutoDelete(TRUE);

    // init the KListView
    setSorting( -1 );
    addColumn( "" );
    setFullWidth(true);
    header()->hide();
    setAcceptDrops(true);
    setDragEnabled(false);
    setDropVisualizer(false);

    //connect the signals
    connect( this, SIGNAL( contextMenuRequested ( QListViewItem *, const QPoint & , int ) ),
             this, SLOT( showPopupMenu(QListViewItem *, const QPoint & , int ) ) );
    connect( this, SIGNAL( doubleClicked (QListViewItem *, const QPoint &, int) ),
             this, SLOT( slotListDblClicked( QListViewItem *, const QPoint &, int) ) );
    connect( this, SIGNAL( dropped(QDropEvent *, QListViewItem *) ),
             this, SLOT( slotDropped(QDropEvent *, QListViewItem *) ) );

    _cfg = NULL;

    QTimer::singleShot(0, this, SLOT(initConfig()));
}

SnippetWidget::~SnippetWidget()
{
  writeConfig();
  delete _cfg;
  _list.clear();
}


/*!
    \fn SnippetWidget::slotAdd()
    Opens the didalog to add a snippet
 */
void SnippetWidget::slotAdd()
{
  kdDebug(26) << "Ender slotAdd()" << endl;
  m_dialog->snippetName->clear();
  m_dialog->snippetText->clear();

  if (m_dialog->exec() == QDialog::Accepted) {
    _list.append( new SnippetItem(this, m_dialog->snippetName->text(), m_dialog->snippetText->text()) );
  }
}


/*!
    \fn SnippetWidget::slotRemove()
    Removes the selected snippet
 */
void SnippetWidget::slotRemove()
{
  //get current data
  QListViewItem * item = currentItem();
  SnippetItem *pSnippet = dynamic_cast<SnippetItem*>( item );
  if (!pSnippet)
    return;

  removeItem(item);
  _list.remove(pSnippet);
}



/*!
    \fn SnippetWidget::slotEdit()
    Opens the dialog of editing the selected snippet
 */
void SnippetWidget::slotEdit()
{
  //get current data
  QListViewItem * item = currentItem();

  SnippetItem *pSnippet = dynamic_cast<SnippetItem*>( item );
  if (!pSnippet)
    return;

  //init the dialog
  m_dialog->snippetName->setText(pSnippet->getName());
  m_dialog->snippetText->setText(pSnippet->getText());
  m_dialog->btnAdd->setText(i18n("&Apply"));

  if (m_dialog->exec() == QDialog::Accepted) {
    //update the KListView and the SnippetItem
    item->setText( 0, m_dialog->snippetName->text() );
    pSnippet->setName( m_dialog->snippetName->text() );
    pSnippet->setText( m_dialog->snippetText->text() );

    setSelected(item, TRUE);
  }

  m_dialog->btnAdd->setText(i18n("Add"));
}


//  \fn SnippetWidget::slotListDblClicked()
/*!
    On a DoubleClick the clicked snippet gets inserted at the
    current cursor position of the active view
*/
void SnippetWidget::slotListDblClicked(QListViewItem * item, const QPoint &, int)
{
  SnippetItem *pSnippet = dynamic_cast<SnippetItem*>( item );
  if (!pSnippet)
      return;

  //process variables if any, then insert into the active view
  insertIntoActiveView( parseText(pSnippet->getText(), _SnippetConfig.getDelimiter()) );
}


/*!
    \fn SnippetWidget::insertIntoActiveView(QString text)
    Inserts the parameter text into the activ view
 */
void SnippetWidget::insertIntoActiveView(QString text)
{
  //get the interfaces for the KTexteditor parts
  KTextEditor::ViewCursorInterface *cursorIface
	= dynamic_cast<KTextEditor::ViewCursorInterface*>(m_part->partController()->activeWidget());

  if (!cursorIface)
    return;

  KTextEditor::EditInterface* editIface
	= dynamic_cast<KTextEditor::EditInterface*>( m_part->partController()->activePart() );

  if (!editIface)
    return;

  uint line, col;
  cursorIface->cursorPositionReal(&line, &col);

  editIface->insertText( line, col , text );
}


/*!
    \fn SnippetWidget::writeConfig()
    Write the cofig file
 */
void SnippetWidget::writeConfig()
{
  _cfg->deleteGroup("SnippetPart");  //this is neccessary otherwise delete entries will stay in list until
                                     //they get overwritten by a more recent entry
  _cfg->setGroup("SnippetPart");

  _cfg->writeEntry("snippetCount", _list.count());
  SnippetItem *item;
  QString strKeyName="";
  QString strKeyText="";
  int iCount = 1;
  for ( item = _list.first(); item; item = _list.next() ) {  //write the snippet-list
    strKeyName=QString("snippetName_%1").arg(iCount);
    strKeyText=QString("snippetText_%1").arg(iCount);

    _cfg->writeEntry(strKeyName, item->getName());
    _cfg->writeEntry(strKeyText, item->getText());

    iCount++;
  }

  iCount = 1;
  QMap<QString, QString>::Iterator it;
  for ( it = _mapSaved.begin(); it != _mapSaved.end(); ++it ) {  //write the saved variable values
    if (it.data().length()<=0) continue;  //is the saved value has no length -> no need to save it

    strKeyName=QString("snippetSavedName_%1").arg(iCount);
    strKeyText=QString("snippetSavedVal_%1").arg(iCount);

    _cfg->writeEntry(strKeyName, it.key());
    _cfg->writeEntry(strKeyText, it.data());

    iCount++;
  }
  _cfg->writeEntry("snippetSavedCount", iCount-1);


  _cfg->writeEntry( "snippetDelimiter", _SnippetConfig.getDelimiter() );
  _cfg->writeEntry( "snippetVarInput", _SnippetConfig.getInputMethod() );
  _cfg->writeEntry( "snippetToolTips", _SnippetConfig.useToolTips() );

  _cfg->writeEntry("snippetSingleRect", _SnippetConfig.getSingleRect() );
  _cfg->writeEntry("snippetMultiRect", _SnippetConfig.getMultiRect() );

  _cfg->sync();
}


/*!
    \fn SnippetWidget::initConfig()
    Initial read the cofig file
 */
void SnippetWidget::initConfig()
{
  if (_cfg == NULL)
    _cfg = new KConfig("kdevsnippetrc", false, false);

  _cfg->setGroup("SnippetPart");

  int iCount = _cfg->readNumEntry("snippetCount", 0);
  QString strKeyName="";
  QString strKeyText="";
  SnippetItem *item;

  for ( int i=1; i<=iCount; i++) {  //read the snippet-list
    strKeyName=QString("snippetName_%1").arg(i);
    strKeyText=QString("snippetText_%1").arg(i);

    QString strNameVal="";
    QString strTextVal="";

    strNameVal = _cfg->readEntry(strKeyName, "");
    strTextVal = _cfg->readEntry(strKeyText, "");

    if (strNameVal != "" && strTextVal != "") {
      item = new SnippetItem(this, strNameVal, strTextVal);
      _list.append(item);
    }
  }


  iCount = _cfg->readNumEntry("snippetSavedCount", 0);

  for ( int i=1; i<=iCount; i++) {  //read the snippet-list
    strKeyName=QString("snippetSavedName_%1").arg(i);
    strKeyText=QString("snippetSavedVal_%1").arg(i);

    QString strNameVal="";
    QString strTextVal="";

    strNameVal = _cfg->readEntry(strKeyName, "");
    strTextVal = _cfg->readEntry(strKeyText, "");

    if (strNameVal != "" && strTextVal != "") {
      _mapSaved[strNameVal] = strTextVal;
    }
  }


  _SnippetConfig.setDelimiter( _cfg->readEntry("snippetDelimiter", "$") );
  _SnippetConfig.setInputMethod( _cfg->readNumEntry("snippetVarInput", 0) );
  _SnippetConfig.setToolTips( _cfg->readBoolEntry("snippetToolTips", true) );

  _SnippetConfig.setSingleRect( _cfg->readRectEntry("snippetSingleRect", 0L) );
  _SnippetConfig.setMultiRect( _cfg->readRectEntry("snippetMultiRect", 0L) );
}

/*!
    \fn SnippetWidget::maybeTip( const QPoint & p )
    Shows the Snippet-Text as ToolTip
 */
void SnippetWidget::maybeTip( const QPoint & p )
{
	SnippetItem * item = dynamic_cast<SnippetItem*>( itemAt( p ) );
	if (!item)
	  return;

	QRect r = itemRect( item );

	if ( item && r.isValid() && _SnippetConfig.useToolTips() )
	{
		tip( r, item->getText() );  //show the tip
	}
}

/*!
    \fn SnippetWidget::showPopupMenu( QListViewItem * item, const QPoint & p, int )
    Shows the Popup-Menu depending item is a valid pointer
*/
void SnippetWidget::showPopupMenu( QListViewItem * item, const QPoint & p, int )
{
	KPopupMenu popup;

	if ( item ) {
		SnippetItem * selectedItem = static_cast<SnippetItem *>(item);

		popup.insertTitle( i18n("Snippet: ") + selectedItem->getName() );

		popup.insertItem( i18n("Add..."), this, SLOT( slotAdd() ) );
		popup.insertItem( i18n("Edit..."), this, SLOT( slotEdit() ) );
		popup.insertItem( i18n("Remove"), this, SLOT( slotRemove() ) );

	} else {
		popup.insertTitle(i18n("Code Snippets"));

		popup.insertItem( i18n("Add..."), this, SLOT( slotAdd() ) );
	}

	popup.exec(p);
}


//  fn SnippetWidget::parseText(QString text, QString del)
/*!
    This function is used to parse the given QString for variables. If found the user will be prompted
    for a replacement value. It returns the string text with all replacements made
 */
QString SnippetWidget::parseText(QString text, QString del)
{
  QString str = text;
  QString strName = "";
  QString strNew = "";
  QString strMsg="";
  int iFound = -1;
  int iEnd = -1;
  QMap<QString, QString> mapVar;
  int iInMeth = _SnippetConfig.getInputMethod();
  QRect rSingle = _SnippetConfig.getSingleRect();
  QRect rMulti = _SnippetConfig.getMultiRect();

  do {
    iFound = text.find(QRegExp("\\"+del+"[A-Z,a-z,\\s]*\\"+del), iEnd+1);  //find the next variable by this QRegExp
    if (iFound >= 0) {
      iEnd = text.find(del, iFound+1)+1;
      strName = text.mid(iFound, iEnd-iFound);

      if ( strName != del+del  &&  mapVar[strName].length() <= 0 ) {  //if not doubel-delimiter and not already in map
        if (iInMeth == 0) {  //check config, if input-method "single" is selected
          strMsg=i18n("Please enter the value for <b>%1</b>:").arg(strName);
          strNew = showSingleVarDialog( strName, &_mapSaved, rSingle );
	  if (strNew=="")
	    return ""; //user clicked Cancle
	}
      } else {
        strNew = del;
      }

      if (iInMeth == 0  ||  strName == del+del) {  //check config, if input-method "single" is selected
        str.replace(strName, strNew);
      } else {
        strNew = strName;
      }

      mapVar[strName] = strNew;
    }
  } while (iFound != -1);

  if (iInMeth == 1) {  //check config, if input-method "multi" is selected
    int w, bh, oh;
    w = rMulti.width();
    bh = rMulti.height();
    oh = rMulti.top();
    if (showMultiVarDialog( &mapVar, &_mapSaved, w, bh, oh )) {  //generate and show the dialog
      QMap<QString, QString>::Iterator it;
      for ( it = mapVar.begin(); it != mapVar.end(); ++it ) {  //walk through the map and do the replacement
        str.replace(it.key(), it.data());
      }
    } else {
      return "";
    }

    rMulti.setWidth(w);   //this is a hack to save the dialog's dimensions in only one QRect
    rMulti.setHeight(bh);
    rMulti.setTop(oh);
    rMulti.setLeft(0);
     _SnippetConfig.setMultiRect(rMulti);
  }

  _SnippetConfig.setSingleRect(rSingle);

  return str;
}


//  fn SnippetWidget::showMultiVarDialog()
/*!
    This function constructs a dialog which contains a label and a linedit for every
    variable that is stored in the given map except the double-delimiter entry
    It return true if everything was ok and false if the user hit cancel
 */
bool SnippetWidget::showMultiVarDialog(QMap<QString, QString> * map, QMap<QString, QString> * mapSave,
                                       int & iWidth, int & iBasicHeight, int & iOneHeight)
{
  //if no var -> no need to show
  if (map->count() == 0)
    return true;

  //if only var is the double-delimiter -> no need to show
  QMap<QString, QString>::Iterator it = map->begin();
  if ( map->count() == 1 && it.data()==_SnippetConfig.getDelimiter()+_SnippetConfig.getDelimiter() )
    return true;

  QMap<QString, KTextEdit *> mapVar2Te;  //this map will help keeping track which TEXTEDIT goes with which variable
  QMap<QString, QCheckBox *> mapVar2Cb;  //this map will help keeping track which CHECKBOX goes with which variable

  // --BEGIN-- building a dynamic dialog
  QDialog dlg(this);
  dlg.setCaption(i18n("Enter Values for Variables"));

  QGridLayout * layout = new QGridLayout( &dlg, 1, 1, 11, 6, "layout");
  QGridLayout * layoutTop = new QGridLayout( 0, 1, 1, 0, 6, "layoutTop");
  QGridLayout * layoutVar = new QGridLayout( 0, 1, 1, 0, 6, "layoutVar");
  QGridLayout * layoutBtn = new QGridLayout( 0, 1, 1, 0, 6, "layoutBtn");

  KTextEdit *te = NULL;
  QLabel * labTop = NULL;
  QCheckBox * cb = NULL;

  labTop = new QLabel( &dlg, "label" );
  labTop->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0,
                         labTop->sizePolicy().hasHeightForWidth() ) );
  labTop->setText(i18n("Enter the replacement values for these variables:"));
  layoutTop->addWidget(labTop, 0, 0);
  layout->addMultiCellLayout( layoutTop, 0, 0, 0, 1 );


  int i = 0;                                           //walk through the variable map and add
  for ( it = map->begin(); it != map->end(); ++it ) {  //a checkbox, a lable and a lineedit to the main layout
    if (it.key() == _SnippetConfig.getDelimiter() + _SnippetConfig.getDelimiter())
      continue;

    cb = new QCheckBox( &dlg, "cbVar" );
    cb->setChecked( FALSE );
    cb->setText(it.key());
    layoutVar->addWidget( cb, i ,0, Qt::AlignTop );

    te = new KTextEdit( &dlg, "teVar" );
    layoutVar->addWidget( te, i, 1, Qt::AlignTop );

    if ((*mapSave)[it.key()].length() > 0) {
      cb->setChecked( TRUE );
      te->setText((*mapSave)[it.key()]);
    }

    mapVar2Te[it.key()] = te;
    mapVar2Cb[it.key()] = cb;

    QToolTip::add( cb, i18n("Enable this to save the value entered to the right as the default value for this variable") );
    QWhatsThis::add( cb, i18n("If you enable this option, the value entered to the right will be saved. "
                              "If you use the same variable later, even in another snippet, the value entered to the right "
			      "will be the default value for that variable.") );

    i++;
  }
  layout->addMultiCellLayout( layoutVar, 1, 1, 0, 1 );

  KPushButton * btn1 = new KPushButton( &dlg, "pushButton1" );
  btn1->setText(i18n("&Cancel"));
  btn1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0,
                         btn1->sizePolicy().hasHeightForWidth() ) );
  layoutBtn->addWidget( btn1, 0, 0 );

  KPushButton * btn2 = new KPushButton( &dlg, "pushButton2" );
  btn2->setText(i18n("&Apply"));
  btn2->setDefault( TRUE );
  btn2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0,
                         btn2->sizePolicy().hasHeightForWidth() ) );
  layoutBtn->addWidget( btn2, 0, 1 );

  layout->addMultiCellLayout( layoutBtn, 2, 2, 0, 1 );
  // --END-- building a dynamic dialog

  //connect the buttons to the QDialog default slots
  connect(btn1, SIGNAL(clicked()), &dlg, SLOT(reject()) );
  connect(btn2, SIGNAL(clicked()), &dlg, SLOT(accept()) );

  //prepare to execute the dialog
  bool bReturn = false;
  //resize the textedits
  if (iWidth > 1) {
    QRect r = dlg.geometry();
    r.setHeight(iBasicHeight + iOneHeight*mapVar2Te.count());
    r.setWidth(iWidth);
    dlg.setGeometry(r);
  }
  if ( dlg.exec() == QDialog::Accepted ) {
    QMap<QString, KTextEdit *>::Iterator it2;
    for ( it2 = mapVar2Te.begin(); it2 != mapVar2Te.end(); ++it2 ) {
      if (it2.key() == _SnippetConfig.getDelimiter() + _SnippetConfig.getDelimiter())
        continue;
      (*map)[it2.key()] = it2.data()->text();    //copy the entered values back to the given map

      if (mapVar2Cb[it2.key()]->isChecked())     //if the checkbox is on; save the values for later
        (*mapSave)[it2.key()] = it2.data()->text();
      else
        (*mapSave).erase(it2.key());
    }
    bReturn = true;

    iBasicHeight = dlg.geometry().height() - layoutVar->geometry().height();
    iOneHeight = layoutVar->geometry().height() / mapVar2Te.count();
    iWidth = dlg.geometry().width();
  }

  //do some cleanup
  QMap<QString, KTextEdit *>::Iterator it1;
  for (it1 = mapVar2Te.begin(); it1 != mapVar2Te.end(); ++it1)
    delete it1.data();
  mapVar2Te.clear();
  QMap<QString, QCheckBox *>::Iterator it2;
  for (it2 = mapVar2Cb.begin(); it2 != mapVar2Cb.end(); ++it2)
    delete it2.data();
  mapVar2Cb.clear();
  delete layoutTop;
  delete layoutVar;
  delete layoutBtn;
  delete layout;

  return bReturn;
}


//  fn SnippetWidget::showSingleVarDialog(QString var, QMap<QString, QString> * mapSave)
/*!
    This function constructs a dialog which contains a label and a linedit for the given variable
    It return either the entered value or an empty string if the user hit cancel
 */
QString SnippetWidget::showSingleVarDialog(QString var, QMap<QString, QString> * mapSave, QRect & dlgSize)
{
  // --BEGIN-- building a dynamic dialog
  QDialog dlg(this);
  dlg.setCaption(i18n("Enter Values for Variables"));

  QGridLayout * layout = new QGridLayout( &dlg, 1, 1, 11, 6, "layout");
  QGridLayout * layoutTop = new QGridLayout( 0, 1, 1, 0, 6, "layoutTop");
  QGridLayout * layoutVar = new QGridLayout( 0, 1, 1, 0, 6, "layoutVar");
  QGridLayout * layoutBtn = new QGridLayout( 0, 2, 1, 0, 6, "layoutBtn");

  KTextEdit *te = NULL;
  QLabel * labTop = NULL;
  QCheckBox * cb = NULL;

  labTop = new QLabel( &dlg, "label" );
  layoutTop->addWidget(labTop, 0, 0);
  labTop->setText((QString(i18n("Enter the replacement values for ")) + var + ":"));
  layout->addMultiCellLayout( layoutTop, 0, 0, 0, 1 );


  cb = new QCheckBox( &dlg, "cbVar" );
  cb->setChecked( FALSE );
  cb->setText("make value &default");

  te = new KTextEdit( &dlg, "teVar" );
  layoutVar->addWidget( te, 0, 1, Qt::AlignTop);
  layoutVar->addWidget( cb, 1, 1, Qt::AlignTop);
  if ((*mapSave)[var].length() > 0) {
    cb->setChecked( TRUE );
    te->setText((*mapSave)[var]);
  }

  QToolTip::add( cb, i18n("Enable this to save the value entered to the right as the default value for this variable") );
  QWhatsThis::add( cb, i18n("If you enable this option, the value entered to the right will be saved. "
                            "If you use the same variable later, even in another snippet, the value entered to the right "
                            "will be the default value for that variable.") );

  layout->addMultiCellLayout( layoutVar, 1, 1, 0, 1 );

  KPushButton * btn1 = new KPushButton( &dlg, "pushButton1" );
  btn1->setText(i18n("&Cancel"));
  layoutBtn->addWidget( btn1, 0, 0 );

  KPushButton * btn2 = new KPushButton( &dlg, "pushButton2" );
  btn2->setText(i18n("&Apply"));
  btn2->setDefault( TRUE );
  layoutBtn->addWidget( btn2, 0, 1 );

  layout->addMultiCellLayout( layoutBtn, 2, 2, 0, 1 );
  te->setFocus();
  // --END-- building a dynamic dialog

  //connect the buttons to the QDialog default slots
  connect(btn1, SIGNAL(clicked()), &dlg, SLOT(reject()) );
  connect(btn2, SIGNAL(clicked()), &dlg, SLOT(accept()) );

  //execute the dialog
  QString strReturn = "";
  if (dlgSize.isValid())
    dlg.setGeometry(dlgSize);
  if ( dlg.exec() == QDialog::Accepted ) {
    if (cb->isChecked())     //if the checkbox is on; save the values for later
      (*mapSave)[var] = te->text();
    else
      (*mapSave).erase(var);

    strReturn = te->text();    //copy the entered values back the the given map

    dlgSize = dlg.geometry();
  }

  //do some cleanup
  delete cb;
  delete te;
  delete labTop;
  delete btn1;
  delete btn2;
  delete layoutTop;
  delete layoutVar;
  delete layoutBtn;
  delete layout;

  return strReturn;
}

//  fn SnippetWidget::acceptDrag (QDropEvent *event) const
/*!
    Reimplementation from KListView.
    Check here if the data the user is about to drop fits our restrictions.
    We only accept dropps of plaintext, because from the dropped text
    we will create a snippet.
 */
bool SnippetWidget::acceptDrag (QDropEvent *event) const
{
  kdDebug(9035) << "Format: " << event->format() << endl;

  if (QString(event->format()).startsWith("text/plain") &&
      static_cast<SnippetWidget *>(event->source()) != this)
    return true;
  else
    return false;
}

//  fn SnippetWidget::slotDropped(QDropEvent *e, QListViewItem *after)
/*!
    This slot is connected to the dropped signal.
    If it is emitted, we need to construct a new snippet entry with
    the data given
 */
void SnippetWidget::slotDropped(QDropEvent *e, QListViewItem *)
{
  QCString dropped;
  QByteArray data = e->encodedData("text/plain");
  if ( e->provides("text/plain") && data.size()>0 ) {
    //get the data from the event...
    QString encData(data.data());
    kdDebug(9035) << "encData: " << encData << endl;

    //... then refill the dialog with the given data
    m_dialog->snippetName->clear();
    m_dialog->snippetText->setText(encData);

    if (m_dialog->exec() == QDialog::Accepted) {
      _list.append( new SnippetItem(this, m_dialog->snippetName->text(), m_dialog->snippetText->text()) );
    }
  }
}


#include "snippet_widget.moc"
