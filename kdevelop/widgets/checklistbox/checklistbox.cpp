/***************************************************************************
                        CheckListBox part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Maximus
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

#include "checklistbox.h"

#include <qapp.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qscrollbar.h>
#include <qtoolbutton.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kfiledialog.h>

#include "checked.xpm"
#include "unchecked.xpm"
#include "notchecked.xpm"
#include "open.xpm"
#include "new_up.xpm"
#include "new_down.xpm"
#include "new_new.xpm"
#include "new_del.xpm"

CBHeader::CBHeader( CheckListBox* parentListBox, const char * name ) :
  QFrame( parentListBox->parentWidget(), name ),
  lb(parentListBox)
{
  setFrameStyle( Panel | Sunken );
  setLineWidth( 1);

  label = new QLabel( this );
  label->setText("Header");

  button_up = new QToolButton( this);
  button_up->setPixmap( new_up_xpm );
  button_up->setFixedSize( 16, 16 );
  connect( button_up, SIGNAL(clicked()), SLOT(slotItemUp()) );

  button_down = new QToolButton( this);
  button_down->setPixmap( new_down_xpm );
  button_down->setFixedSize( 16, 16 );
  connect( button_down, SIGNAL(clicked()), SLOT(slotItemDown()) );

  button_new = new QToolButton( this);
  button_new->setPixmap( new_new_xpm );
  button_new->setFixedSize( 16, 16 );
  connect( button_new, SIGNAL(clicked()), SLOT(slotNewItem()) );

  button_del = new QToolButton( this);
  button_del->setPixmap( new_del_xpm );
  button_del->setFixedSize( 16, 16 );
  connect( button_del, SIGNAL(clicked()), SLOT(slotDelItem()) );

  l = new QHBoxLayout( this, 2 );
  l->addWidget( label );
  l->addWidget( button_new );
  l->addWidget( button_up );
  l->addWidget( button_down );
  l->addWidget( button_del );
  l->activate();
}

CBHeader::~CBHeader()
{
}

void CBHeader::updateGeometry()
{
  setFixedSize( lb->width(), 20 );
  move( lb->x(), lb->y() - 22);
}

void CBHeader::setCaption( QString text)
{
  label->setText( text);
}

QString CBHeader::getCaption()
{
  return label->text();
}

void CBHeader::slotNewItem()
{
  if ( lb->isAllowEdit() ){
    int index = lb->count();
    lb->insertItem( "new", -1 );
    lb->setCurrentItem( index );
    lb->setBottomItem( index );
    lb->slotSelected( index );
  }
}

void CBHeader::slotDelItem()
{
  if ( lb->currentItem() == -1 ) return;
  if ( lb->item(lb->currentItem())->isEnableDeleted() )
    lb->removeItem( lb->currentItem() );
}

void CBHeader::slotItemUp()
{
  int c  = lb->currentItem();
  if ( c == -1 || c == 0 ) return;
  CheckListBoxItem* tempItem = new CheckListBoxItem(*lb->item(c));
  lb->setAutoUpdate(false);
  lb->removeItem(c);
  lb->insertCBItem( tempItem, c - 1);
  lb->setCurrentItem( c - 1 );
  lb->setAutoUpdate(true);
  lb->repaint(false);
}

void CBHeader::slotItemDown()
{
  int c  = lb->currentItem();
  if ( c == -1 || c == (int)lb->count()-1 ) return;
  CheckListBoxItem* tempItem = new CheckListBoxItem(*lb->item(c));
  lb->setAutoUpdate(false);
  lb->removeItem(c);
  lb->insertCBItem( tempItem, c + 1);
  lb->setCurrentItem( c + 1 );
  lb->setAutoUpdate(true);
  lb->repaint(false);
}

CBLineEdit::CBLineEdit( QWidget * parent, const char * name )
:QLineEdit( parent, name )
{
}

CBLineEdit::~CBLineEdit()
{
}

void CBLineEdit::focusOutEvent( QFocusEvent * )
{
  emit ignore();
}

void CBLineEdit::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Key_Escape ) emit ignore();
  QLineEdit::keyPressEvent( e );
  e->accept();
}

CheckListBoxItem::CheckListBoxItem( const char * text, bool checked, CheckListBox* parent )
: QListBoxText(),
  chPix( parent->checkPix ),
  unchPix( parent->unCheckPix ),
  notchPix( parent->notCheckPix ),
  pix( parent->defPix ),
  actionPix( parent->defActionPix ),
  actionText( parent->defActionText ),
  actionShowText( parent->defActionShowText ),
  actionShowPix( parent->defActionShowPix ),
  enableDelete( true ),
  lb( parent ),
  ch( checked ),
  _allowEdit( parent->allowEdit ),
  _allowCheck( parent->allowCheck ),
  _allowAction( parent->allowAction )
{
  setText( text );
}

CheckListBoxItem::CheckListBoxItem( const CheckListBoxItem & i )
: QListBoxText(),
  chPix( i.chPix ),
  unchPix( i.unchPix ),
  notchPix( i.notchPix ),
  pix( i.pix ),
  actionPix( i.actionPix ),
  actionText( i.actionText ),
  actionShowText( i.actionShowText ),
  actionShowPix( i.actionShowPix ),
  enableDelete( i.enableDelete ),
  lb( i.lb ),
  ch( i.ch ),
  _allowEdit( i._allowEdit ),
  _allowCheck( i._allowCheck ),
  _allowAction( i._allowAction )
{
  setText( i.text() );
}

int CheckListBox::getXstartTextPaint()
{
  int x1 = allowCheck ? 16:0;
  int x2 = showIcon   ? 16:0;
  return  2+ x1 + x2;
}

void CheckListBoxItem::paint ( QPainter* p )
{
  int d = lb->getXstartTextPaint();

	QFontMetrics fm = p->fontMetrics();
	p->drawText( d ,  fm.ascent() + fm.leading()/2 + 1, text() );

  if ( lb->allowCheck ){

    int y1 = ( height() - lb->imList[chPix]->height() )/2;
    int y2 = ( height() - lb->imList[unchPix]->height() )/2;
    int y3 = ( height() - lb->imList[notchPix]->height() )/2;

    if ( _allowCheck ){
      if ( ch )
        p->drawPixmap(2, y1, *lb->imList[chPix] );
      else
        p->drawPixmap(2, y2, *lb->imList[unchPix] );
      } else
        p->drawPixmap(2, y3, *lb->imList[notchPix] );
  }
  if ( (lb->showIcon) && (pix != "") && (lb->imList[pix] != 0L) ){

    int y4 = ( height() - lb->imList[pix]->height() )/2;
    p->drawPixmap( lb->allowCheck ? 16:2 , y4, *lb->imList[pix] );
  }
}

int CheckListBoxItem::width() const
{
  return QListBoxText::width(lb) + lb->getXstartTextPaint();
}

int CheckListBoxItem::height() const
{
  int h = lb->fontMetrics().lineSpacing() + 2;
  return QMAX( 16, h );
}

CheckListBoxItem::~CheckListBoxItem()
{
}

/////////////////////////////////////////////////////////////////////////////////

CheckListBox::CheckListBox( QWidget * parent=0, const char * name=0, WFlags f=0 )
:QListBox( parent,name,f )
{
  showH = false;
  hPosAuto = true;

  allowCheck  = true;
  allowEdit   = true;
  allowAction = true;
  showIcon    = false;

  getItemText = 0L;

  checkPix = "CHECK";
  unCheckPix = "UNCHECK";
  notCheckPix = "NOTCHECK";
  defPix = "";
  defActionPix = "AC_OPEN";

  defActionText = "...";
  defActionShowText = false;
  defActionShowPix = true;

  imList.insert( checkPix, new QPixmap( checked_xpm ) );
  imList.insert( unCheckPix, new QPixmap( unchecked_xpm ) );
  imList.insert( notCheckPix, new QPixmap( notchecked_xpm ) );
  imList.insert( defActionPix, new QPixmap( open_xpm ) );

  connect( this, SIGNAL(selected(int)), SLOT(slotSelected(int)) );

  frame = new QFrame( this );
  frame->setFrameStyle( Box | Sunken );
  frame->setLineWidth( 1);

  frame->hide();

  edit = new CBLineEdit( frame );
  edit->setFrame(QFrame::NoFrame);

  connect( edit, SIGNAL(ignore()), SLOT(slotEditIgnore()) );
  connect( edit, SIGNAL(returnPressed()), SLOT(slotEditAccept()) );

  connect( (QObject*)verticalScrollBar(), SIGNAL(sliderPressed()), SLOT(slotEditIgnore()) );
  connect( (QObject*)verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(slotEditIgnore()) );

  button = new QToolButton( frame );
  connect( button, SIGNAL(clicked()), SLOT(slotButtonClick()) );

  lastDir = "/";
  header = 0L;
}

CheckListBox::~CheckListBox()
{
}

CheckListBoxItem* CheckListBox::insertItem ( const char * text, bool checked, int index )
{
  CheckListBoxItem* item = new CheckListBoxItem( text, checked, this );
  QListBox::insertItem( item, index );
  return item;
}

void CheckListBox::mousePressEvent( QMouseEvent *e )
{
  if ( (e->x() < 18 ) && allowCheck ){
    int id = findItem( e->y() );
    if ( id != -1 )
      if ( item(id)->_allowCheck ){
        item(id)->ch = !(item(id)->ch);
        if ( autoUpdate() ) repaint();
        emit itemCheckChange( id, item(id)->ch );
      }
  }
  QListBox::mousePressEvent( e );
}

void CheckListBox::paintCell( QPainter *p, int row, int col )
{
  if ( item(row) == 0L ) return;

  QColorGroup g = colorGroup();
  if ( isSelected( row ) ) {
    p->fillRect( 0, 0, cellWidth(col), cellHeight(row), QApplication::winStyleHighlightColor() );
    p->setPen( white );
    p->setBackgroundColor( g.text() );
  } else {
    p->fillRect( 0, 0, cellWidth(col), cellHeight(row), g.base() );
    p->setBackgroundColor( g.base() );
    p->setPen( g.text() );
  }
  item( row )->paint( p );

  p->setPen( g.text() );
 	p->setBackgroundColor( g.base() );
}

void CheckListBox::setDefCheckPixName( QString name, bool applyAll )
{
  checkPix = name.isNull() ? QString("CHECK") : name;
  if ( applyAll )
    for ( uint k = 0; k < count(); k++ ) item( k )->chPix = checkPix;
  if ( autoUpdate() ) repaint();
}

void CheckListBox::setDefItemActionPixName( QString name, bool applyAll )
{
  defActionPix = name;
  if ( applyAll )
    for ( uint k = 0; k < count(); k++ ) item( k )->chPix = defActionPix;
  if ( autoUpdate() ) repaint();
}

void CheckListBox::setDefUnCheckPixName( QString name, bool applyAll )
{
  unCheckPix = name.isNull() ? QString("UNCHECK") : name;
  if ( applyAll )
    for ( uint k = 0; k < count(); k++ ) item( k )->chPix = unCheckPix;
  if ( autoUpdate() ) repaint();
}

void CheckListBox::setDefItemPixName( QString name, bool applyAll )
{
  defPix = name;
  if ( applyAll )
    for ( uint k = 0; k < count(); k++ ) item( k )->pix = defPix;
  if ( autoUpdate() ) repaint();
}

void CheckListBox::setDefNotCheckPixName( QString name, bool applyAll )
{
  notCheckPix = name.isNull() ? QString("NOTCHECK") : name;
  if ( applyAll )
    for ( uint k = 0; k < count(); k++ ) item( k )->chPix = notCheckPix;
  if ( autoUpdate() ) repaint();
}

void CheckListBox::allowChecked( bool ch )
{
  if ( ch == allowCheck ) return;
  allowCheck = ch;
  if ( autoUpdate() ) repaint();
}

void CheckListBox::slotSelected( int id )
{
  if ( !(item(id)->_allowEdit) ) return;

  int y;
  itemYPos( id, &y );
  frame->setGeometry(contentsRect().left(), y-2,
                     contentsRect().width(), cellHeight(id)+4);


  QRect r = frame->contentsRect();

  if ( item(id)->_allowAction ){
    button->setTextLabel( item(id)->actionText );
    button->setUsesTextLabel( item(id)->actionShowText );
    button->setPixmap( imList[item(id)->actionPix] ? *imList[item(id)->actionPix] : QPixmap("") );
    //    if ( !item(id)->actionShowPix ) button->setPixmap("");
    button->show();
    button->setGeometry( QRect( QPoint(r.width() - button->sizeHint().width(), r.top()),
                                QSize(button->sizeHint().width(), r.height()) ));
    edit->setGeometry( QRect( r.topLeft(), QPoint(r.width() - button->width()-4, r.height()) ) );
  } else {
    edit->setGeometry( QRect( r.topLeft(), QPoint(r.width()-4, r.height()) ) );
  }

  edit->setText( text(id) );
  edit->setFont(font());
  edit->selectAll();
  frame->show();
  edit->setFocus();
}

void CheckListBox::slotEditIgnore()
{
  int c = currentItem();
  setFocus();
  if ( QString( text(c) ).length() == 0 ) removeItem(c);
  frame->hide();
}

void CheckListBox::slotEditAccept()
{
  int c = currentItem();
  QString oldText = text(c);
  item(c)->CLBsetText( edit->text() );
  setFocus();
  frame->hide();
  if ( QString( text(c) ).length() == 0 ) removeItem(c);
  else
    emit itemTextChange( c , edit->text(), oldText);
}

void CheckListBox::slotButtonClick()
{
  QString newText = "";
  if ( getItemText == 0L ){
    QString curDir = text( currentItem() );
    if ( !QDir(curDir).exists() ) curDir = lastDir.data();
    newText = KFileDialog::getDirectory( curDir, this );
  } else {
    newText =(this->*getItemText)( currentItem() );
  }
  if ( newText.isEmpty() ) return;
  edit->setText( newText);
  slotEditAccept();
  lastDir = newText;
}

CheckListBoxItem* CheckListBox::item( int index )
{
  return (CheckListBoxItem*)(QListBox::item(index));
}

bool CheckListBoxItem::isChecked()
{
  return ch;
}

void CheckListBoxItem::setCheck( bool isChecked )
{
  if ( ch == isChecked ) return;
  ch = isChecked;
  for ( uint k = 0; k < lb->count(); k++ )
    if ( lb->item(k) == this ) emit lb->itemCheckChange( k, ch );
  if ( lb->autoUpdate() ) lb->repaint();
}

void CheckListBoxItem::allowChecked( bool allow, bool def = false )
{
  ch = def;
  _allowCheck = allow;
  if ( lb->autoUpdate() ) lb->repaint();
}

bool CheckListBoxItem::isAllowChecked()
{
  return _allowCheck;
}

void CheckListBoxItem::allowEdit( bool allow )
{
  _allowEdit = allow;
}

bool CheckListBoxItem::isAllowEdit()
{
  return _allowEdit;
}

void CheckListBoxItem::allowAction( bool allow )
{
  _allowAction = allow;
}

bool CheckListBoxItem::isAllowAction()
{
  return _allowAction;
}

void CheckListBox::addImageToList( QString name, QPixmap pix )
{
  imList.insert( name, new QPixmap( pix ) );
  if ( autoUpdate() ) repaint();
}

void CheckListBox::replaceImageInList( QString name, QPixmap pix )
{
  imList.replace( name, new QPixmap( pix ) );
  if ( autoUpdate() ) repaint();
}

void CheckListBoxItem::setCheckPixName( QString name )
{
  chPix = name;
  if ( lb->autoUpdate() ) lb->repaint();
}

void CheckListBoxItem::setUnCheckPixName( QString name )
{
  unchPix = name;
  if ( lb->autoUpdate() ) lb->repaint();
}

void CheckListBoxItem::setNotCheckPixName( QString name )
{
  notchPix = name;
  if ( lb->autoUpdate() ) lb->repaint();
}

void CheckListBoxItem::setItemPixName( QString name )
{
  pix = name;
  if ( lb->autoUpdate() ) lb->repaint();
}

void CheckListBox::allowIconShow( bool allow )
{
  showIcon = allow;
  if ( autoUpdate() ) repaint();
}

void CheckListBox::resizeEvent( QResizeEvent* e )
{
  QListBox::resizeEvent( e );
  if ( showH && hPosAuto ){
    QListBox:setGeometry(x(), y()+25, width(), height()-25);
    header->updateGeometry();
  }
}

CBHeader* CheckListBox::getHeaderWidget() const
{
 return showH ? header : 0L;
}

void CheckListBox::showHeader( bool show )
{
  if ( show == showH ) return;
  showH = show;
  if ( showH ){
    header = new CBHeader(this, "CBHeader");
    header->updateGeometry();
  } else {
    delete header;
    header = 0L;
  }
}

void CheckListBox::insertCBItem ( CheckListBoxItem* item , int index )
{
  QListBox::insertItem( item, index );
}





























