/***************************************************************************
                          kdlgproplvis.h  -  description
                             -------------------
    begin                : Wed Mar 17 1999
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef KDLGPROPLVIS_H
#define KDLGPROPLVIS_H

#include <qwidget.h>
#include <qdialog.h>

/**
  *@author Pascal Krahmer
  */

#include "defines.h"


class AdvLvi_Base;
class QPushButton;
class KDlgItem_Base;
class QButton;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QLabel;
class KDlgPropertyEntry;
class KRestrictedLine;
class KColorButton;
class CKDevelop;
class QGroupBox;
class QMultiLineEdit;
class QToolButton;

class AdvLvi_Base : public QWidget
{
  Q_OBJECT
  public:
    AdvLvi_Base(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText() { return QString(); }
    virtual QString getDisplayedText() { return QString(); }
    virtual void refreshItem();

  protected:
    virtual void paintEvent ( QPaintEvent * );
    KDlgPropertyEntry *propEntry;

    CKDevelop *pCKDevel;
};


class AdvLvi_String : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_String(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText();


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QLineEdit *leInput;

  public slots:
    void returnPressed();
};

class AdvLvi_Int : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_Int(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText();

  protected:
    virtual void resizeEvent ( QResizeEvent * );

    KRestrictedLine *leInput;
    QPushButton *up, *down;

  public slots:
    void returnPressed();
    void valInc();
    void valDec();
};


class AdvLvi_UInt : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_UInt(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText();

  protected:
    virtual void resizeEvent ( QResizeEvent * );

    KRestrictedLine *leInput;
    QPushButton *up, *down;

  public slots:
    void returnPressed();
    void valInc();
    void valDec();
};

class AdvLvi_ExtEdit : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_ExtEdit(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText();


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QButton *btnMore;
    QLineEdit *leInput;

  public slots:
    void returnPressed();
};

class AdvLvi_Bool : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_Bool(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText();


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QComboBox *cbBool;
//    QCheckBox *cbBool;
  public slots:
//    void btnPressed();
  void activated( const QString & );
};


class AdvLvi_Orientation : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_Orientation(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText();


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QComboBox *cbOrientation;

  public slots:
    void activated( const QString & );
};

class AdvLvi_Filename : public AdvLvi_ExtEdit
{
  Q_OBJECT
  public:
    AdvLvi_Filename(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0 );

  public slots:
    void btnPressed();
};

class AdvLvi_Font : public AdvLvi_ExtEdit
{
  Q_OBJECT
  public:
    AdvLvi_Font(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0 );

  public slots:
    void btnPressed();
};

class AdvLvi_ColorEdit : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_ColorEdit(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0 );

    virtual QString getText();

  protected:
    virtual void resizeEvent ( QResizeEvent * );
    KColorButton *btn;
    QLineEdit *leInput;

  public slots:
    void returnPressed();
    void changed ( const QColor &newColor );
};

class AdvLvi_BgMode : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_BgMode(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText();


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QComboBox *cbBool;

  public slots:
    void activated( const QString & );
};

class AdvLvi_Cursor : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_Cursor(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText();


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QComboBox *cbBool;
    QButton *btnMore;

  public slots:
    void activated( const QString & );
    void btnPressed();
};



class AdvLvi_Varname : public AdvLvi_String
{
  Q_OBJECT
  public:
    AdvLvi_Varname(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

  public slots:
    void VarnameChanged();
};

class AdvLvi_ComboList : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_ComboList(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText();


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QComboBox *cbOrientation;

  public slots:
    void activated( const QString & );
};


class dlgMultiLineEdit : public QDialog
{
     Q_OBJECT
  public:
      dlgMultiLineEdit(QWidget *parent=0, const char *name=0);

      QString getText();
  protected:
      void initDialog();
      QGroupBox *gbGroupBox;
      QMultiLineEdit *mleStrings;
      QLabel *lRowCnt;
      QPushButton *btnCancel;
      QPushButton *btnOk;
  private:
  public slots:
      void textChanged( );
};

class AdvLvi_MultiString : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_MultiString(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText();
    virtual QString getDisplayedText() { return QString("[QStrings...]"); }


  protected:
    QString txt;
    QToolButton *btnMore;

    virtual void resizeEvent ( QResizeEvent * );

  public slots:
    void btnPressed();
};
#endif
