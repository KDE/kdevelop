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



class AdvLvi_Base : public QWidget
{
  Q_OBJECT
  public:
    AdvLvi_Base(QWidget *parent=0, const char *name=0);

    virtual QString getText() { return QString(); }

  protected:
    virtual void paintEvent ( QPaintEvent * );
};


class AdvLvi_ExtEdit : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_ExtEdit(QWidget *parent=0, const char *name=0);

    virtual QString getText();


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QButton *btnMore;
    QLineEdit *leInput;
};

class AdvLvi_Bool : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_Bool(QWidget *parent=0, const char *name=0);

    virtual QString getText();


  protected:
    virtual void resizeEvent ( QResizeEvent * );

    QComboBox *cbBool;
};

class AdvLvi_Filename : public AdvLvi_ExtEdit
{
  Q_OBJECT
  public:
    AdvLvi_Filename(QWidget *parent=0, const char *name=0 );

  public slots:
    void btnPressed();
};

class KColorButton;

class AdvLvi_ColorEdit : public AdvLvi_Base
{
  Q_OBJECT
  public:
    AdvLvi_ColorEdit(QWidget *parent=0, const char *name=0 );

    virtual QString getText();

  protected:
    virtual void resizeEvent ( QResizeEvent * );
    KColorButton *btn;
};





#endif
