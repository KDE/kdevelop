/***************************************************************************
                          GppCompilerOptionsWidget.h  -  description
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

#ifndef CPPCOMPILEROPTIONSWIDGET_H
#define CPPCOMPILEROPTIONSWIDGET_H

#include <qwidget.h>
#include <GppCompilerOptionsWidgetBase.h>
#include "flagboxes.h"
#include <qradiobutton.h>
#include <qvaluelist.h>
#include <qtabwidget.h>
#include <kdialogbase.h>
#include "KDevCompiler.h"
/**
  *@author Omid Givi
  */

class GeneralTabGpp;
class CodeGenTabGpp;
class OptimizationTabGpp;
class Warnings1TabGpp;
class Warnings2TabGpp;

class GppCompilerOptionsWidget : public GppCompilerOptionsWidgetBase{
   Q_OBJECT
public:
	GppCompilerOptionsWidget(QWidget *parent=0, const char *name=0, QWidget *pdlg=0);
	~GppCompilerOptionsWidget();
	void ButtonApplyClicked();

public slots:
    void slotFlagsToolButtonClicked();
    void slotClearAllClicked();
    void slotWidgetStarted(KDevCompiler *kdc);
    void slotButtonApplyClicked(KDevCompiler *kdc);
};
  class GeneralTabGpp : public QWidget{
  public:
    GeneralTabGpp(QWidget *parent=0, const char *name=0 );
    ~GeneralTabGpp();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagCheckBoxController *controller;
  };


  class CodeGenTabGpp : public QWidget{
  public:
    CodeGenTabGpp(QWidget *parent=0, const char *name=0 );
    ~CodeGenTabGpp();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagListBox *genBox;

  };


  class OptimizationTabGpp : public QWidget{
  public:
    OptimizationTabGpp(QWidget *parent=0, const char *name=0 );
    ~OptimizationTabGpp();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    QRadioButton *Odefault, *O0, *O1, *O2;
    FlagListBox *optBox;
  };


  class Warnings1TabGpp : public QWidget{
  public:
    Warnings1TabGpp(QWidget *parent=0, const char *name=0 );
    ~Warnings1TabGpp();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagCheckBoxController *controller;
    FlagListBox *wallBox;
  };


  class Warnings2TabGpp : public QWidget{
  public:
    Warnings2TabGpp(QWidget *parent=0, const char *name=0 );
    ~Warnings2TabGpp();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagListBox *wrestBox;
  };

  class GppOptionsDlg : public KDialogBase{
    Q_OBJECT

  public:
    GppOptionsDlg(QWidget *parent=0, const char *name=0 );
    ~GppOptionsDlg();
    QString flags() const;
    QString setFlags(const QString &flags);

  private:
    GeneralTabGpp *general;
    CodeGenTabGpp *codegen;
    OptimizationTabGpp *optimization;
    Warnings1TabGpp *warnings1;
    Warnings2TabGpp *warnings2;
  };
#endif
