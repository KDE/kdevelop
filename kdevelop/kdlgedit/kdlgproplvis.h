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
class KDlgPropertyEntry;
class KRestrictedLine;
class KColorButton;
class CKDevelop;

class AdvLvi_Base : public QWidget
{
  Q_OBJECT
  public:
    AdvLvi_Base(QWidget *parent, CKDevelop *parCKD, KDlgPropertyEntry *dpe, const char *name=0);

    virtual QString getText() { return QString(); }
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

  public slots:
    void activated( const char* );
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

  public slots:
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
    void activated( const char* );
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
    void activated( const char* );
    void btnPressed();
};


#endif
