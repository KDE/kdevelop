/***************************************************************************
                        CheckListBox part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Maximus
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

#ifndef CHECKLISTBOX_H
#define CHECKLISTBOX_H

#include <qframe.h>
#include <qlistbox.h>
#include <qdict.h>
#include <qstring.h>
#include <qlineedit.h>

typedef QDict<QPixmap> CBImageList;

class QPainter;
class CheckListBox;
class QToolButton;
class QFrame;
class QLabel;
class QHBoxLayout;


#define CALLBACK_ACTION(FUNC) setActionFunction((CLBcallBackFunc)&FUNC)

class CBHeader : public QFrame
{Q_OBJECT
friend CheckListBox;
public:
  void setCaption( QString text);
  QString getCaption();

private slots:
  void slotNewItem();
  void slotDelItem();
  void slotItemUp();
  void slotItemDown();

private:
  CBHeader( CheckListBox* parentListBox, const char * name=0 );
  ~CBHeader();

  void updateGeometry();

  QHBoxLayout* l;
  CheckListBox* lb;
  QLabel* label;
  QToolButton* button_up;
  QToolButton* button_down;
  QToolButton* button_new;
  QToolButton* button_del;
};
/*========================================================*/

class CBLineEdit : public QLineEdit
{Q_OBJECT
friend CheckListBox;

  CBLineEdit( QWidget * parent=0, const char * name=0 );
  ~CBLineEdit();

  void keyPressEvent( QKeyEvent * );
  void focusOutEvent ( QFocusEvent * );

signals:
  void ignore();
};
/*========================================================*/

class CheckListBoxItem : public QListBoxText
{
  friend CheckListBox;
public:
  CheckListBoxItem( const char*, bool, CheckListBox* );
  CheckListBoxItem( const CheckListBoxItem & );
  ~CheckListBoxItem();

  void setCheck( bool isChecked = true );
  bool isChecked();

  void allowChecked( bool allow, bool def = false );
  bool isAllowChecked();

  void allowEdit( bool allow );
  bool isAllowEdit();

  void allowAction( bool allow );
  bool isAllowAction();

  void setCheckPixName   ( QString );
  void setUnCheckPixName ( QString );
  void setNotCheckPixName( QString );
  void setItemPixName    ( QString );
  void setActionPixName  ( QString name ){ actionPix = name; };

  QString getCheckPixName()   { return chPix;     }
  QString getUnCheckPixName() { return unchPix;   }
  QString getNotCheckPixName(){ return notchPix;  }
  QString getItemPixName()    { return pix;       }
  QString getActionPixName()  { return actionPix; }

  QString getActionText(){ return actionText; }
  bool    isActionShowText() { return actionShowText; }
  bool    isActionShowPix () { return actionShowPix;  }

  void setActionText( QString text ){ actionText = text; }
  void setActionShowText( bool f) { actionShowText = f; }
  void setActionShowPix ( bool f) { actionShowPix  = f; }

  void enableDeleted ( bool f) { enableDelete  = f; }
  bool isEnableDeleted () { return enableDelete; }

  // any user definity data
  int userData;

protected:
  virtual int width()  const;
  virtual int height() const;

private:
  QString chPix;
  QString unchPix;
  QString notchPix;
  QString pix;
  QString actionPix;

  QString actionText;
  bool    actionShowText;
  bool    actionShowPix;
  bool    enableDelete;

  CheckListBox* lb;
  bool ch;
  bool _allowEdit;
  bool _allowCheck;
  bool _allowAction;
  void paint( QPainter* );
  void CLBsetText( const char* t ){ setText(t); }
};
/*========================================================*/

class CheckListBox : public QListBox
{Q_OBJECT
  friend CheckListBoxItem;
  friend CBHeader;
public:
  CheckListBox( QWidget * parent=0, const char * name=0, WFlags f=0 );
  ~CheckListBox();

  /* DO NO DELETE ITSELF THIS ITEM
     this is only for set any property
  */
  CheckListBoxItem* insertItem ( const char * text, bool checked = false, int index=-1 );
  CheckListBoxItem* item( int index );

  void setDefCheckPixName      ( QString, bool applyAll = false );
  void setDefUnCheckPixName    ( QString, bool applyAll = false );
  void setDefNotCheckPixName   ( QString, bool applyAll = false );
  void setDefItemPixName       ( QString, bool applyAll = false );
  void setDefItemActionPixName ( QString, bool applyAll = false );

  QString getDefCheckPixName()      { return checkPix;    }
  QString getDefUnCheckPixName()    { return unCheckPix;  }
  QString getDefNotCheckPixName()   { return notCheckPix; }
  QString getDefItemPixName()       { return defPix;      }
  QString getDefItemActionPixName() { return defActionPix;}

  typedef QString(QObject::*CLBcallBackFunc)(int);
  void setActionFunction( CLBcallBackFunc f ){ getItemText = f; };

  bool isAllowChecked(){ return allowCheck;  }
  bool isAllowEdit()   { return allowEdit;   }
  bool isAllowAction() { return allowAction; }
  bool isIconShow()    { return showIcon;    }
  void allowChecked ( bool );
  void allowEditing ( bool allow ){ allowEdit   = allow; }
  void allowActions ( bool allow ){ allowAction = allow; }
  void allowIconShow( bool );

  CBImageList getImageList() const { return imList; }
  void addImageToList( QString name, QPixmap pix );
  void replaceImageInList( QString name, QPixmap pix );

  QString getDefActionText(){ return defActionText; }
  bool    isDefActionShowText() { return defActionShowText; }
  bool    isDefActionShowPix () { return defActionShowPix;  }

  void setDefActionText( QString text ){ defActionText = text; }
  void setDefActionShowText( bool f) { defActionShowText = f; }
  void setDefActionShowPix ( bool f) { defActionShowPix  = f; }

  CBHeader* getHeaderWidget() const;
  void showHeader( bool );
  void headerPosAuto( bool hauto ){ hPosAuto = hauto; }
  bool isHeaderPosAuto(){ return hPosAuto; }
  bool isShowHeader(){ return showH; }

protected:
  virtual void mousePressEvent ( QMouseEvent * );
  virtual void paintCell( QPainter*, int, int );
  virtual void resizeEvent( QResizeEvent * );
  int getXstartTextPaint();

private:
  bool allowCheck;
  bool allowEdit;
  bool allowAction;
  bool showIcon;

  bool showH;
  bool hPosAuto;

  QString checkPix;
  QString unCheckPix;
  QString notCheckPix;
  QString defPix;
  QString defActionPix;

  QString defActionText;
  bool    defActionShowText;
  bool    defActionShowPix;

  CBHeader* header;
  CBLineEdit* edit;
  QToolButton* button;
  QFrame* frame;
  QString lastDir;

  CLBcallBackFunc getItemText;
  CBImageList imList;

  void insertCBItem ( CheckListBoxItem*, int );

private slots:
  void slotSelected(int);
  void slotEditIgnore();
  void slotEditAccept();
  void slotButtonClick();

signals:
  void itemTextChange( int, const char*, const char* );
  void itemCheckChange( int, bool );
};

#endif
