/***************************************************************************
                          GccCompilerOptionsWidget.h  -  description
                             -------------------
    begin                : Mon Feb 5 2001
    copyright            : (C) 2001 by Omid Givi & Bernd Gehrmann
    email                : omid@givi.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GCCCOMPILEROPTIONSWIDGET_H
#define GCCCOMPILEROPTIONSWIDGET_H

#include <qwidget.h>
#include <GccCompilerOptionsWidgetBase.h>
#include "flagboxes.h"
#include <qradiobutton.h>
#include <qvaluelist.h>
#include <qtabwidget.h>
#include <kdialogbase.h>
#include "KDevCompiler.h"

/**
  *@author Omid Givi
  */

class GeneralTabGcc;
class CodeGenTabGcc;
class OptimizationTabGcc;
class Warnings1TabGcc;
class Warnings2TabGcc;

class GccCompilerOptionsWidget : public GccCompilerOptionsWidgetBase{
   Q_OBJECT
public:
	GccCompilerOptionsWidget(QWidget *parent=0, const char *name=0, QWidget *pdlg=0);
	~GccCompilerOptionsWidget();

public slots:
    void slotFlagsToolButtonClicked();
    void slotClearAllClicked();
    void slotWidgetStarted(KDevCompiler *kdc);
    void slotButtonApplyClicked(KDevCompiler *kdc);
};

  class GeneralTabGcc : public QWidget{
  public:
    GeneralTabGcc(QWidget *parent=0, const char *name=0 );
    ~GeneralTabGcc();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagCheckBoxController *controller;
  };


  class CodeGenTabGcc : public QWidget{
  public:
    CodeGenTabGcc(QWidget *parent=0, const char *name=0 );
    ~CodeGenTabGcc();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagListBox *genBox;

  };


  class OptimizationTabGcc : public QWidget{
  public:
    OptimizationTabGcc(QWidget *parent=0, const char *name=0 );
    ~OptimizationTabGcc();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    QRadioButton *Odefault, *O0, *O1, *O2;
    FlagListBox *optBox;
  };


  class Warnings1TabGcc : public QWidget{
  public:
    Warnings1TabGcc(QWidget *parent=0, const char *name=0 );
    ~Warnings1TabGcc();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagCheckBoxController *controller;
    FlagListBox *wallBox;
  };


  class Warnings2TabGcc : public QWidget{
  public:
    Warnings2TabGcc(QWidget *parent=0, const char *name=0 );
    ~Warnings2TabGcc();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagListBox *wrestBox;
  };

  class GccOptionsDlg : public KDialogBase{
    Q_OBJECT

  public:
    GccOptionsDlg(QWidget *parent=0, const char *name=0 );
    ~GccOptionsDlg();
    QString flags() const;
    QString setFlags(const QString &flags);

  private:
    GeneralTabGcc *general;
    CodeGenTabGcc *codegen;
    OptimizationTabGcc *optimization;
    Warnings1TabGcc *warnings1;
    Warnings2TabGcc *warnings2;
  };

#endif
