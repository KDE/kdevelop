/***************************************************************************
                          CppCompilerOptionsWidget.h  -  description
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
#include <CppCompilerOptionsWidgetBase.h>
#include "flagboxes.h"
#include <qradiobutton.h>
#include <qvaluelist.h>
#include <qtabwidget.h>
#include <kdialogbase.h>
#include "KDevCompiler.h"
/**
  *@author Omid Givi
  */

class GeneralTabCpp;
class CodeGenTabCpp;
class OptimizationTabCpp;
class Warnings1TabCpp;
class Warnings2TabCpp;

class CppCompilerOptionsWidget : public CppCompilerOptionsWidgetBase{
   Q_OBJECT
public:
	CppCompilerOptionsWidget(QWidget *parent=0, const char *name=0, QWidget *pdlg=0);
	~CppCompilerOptionsWidget();
	void ButtonApplyClicked();

public slots:
    void slotFlagsToolButtonClicked();
    void slotClearAllClicked();
    void slotWidgetStarted(KDevCompiler *kdc);
    void slotButtonApplyClicked(KDevCompiler *kdc);
};
  class GeneralTabCpp : public QWidget{
  public:
    GeneralTabCpp(QWidget *parent=0, const char *name=0 );
    ~GeneralTabCpp();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagCheckBoxController *controller;
  };


  class CodeGenTabCpp : public QWidget{
  public:
    CodeGenTabCpp(QWidget *parent=0, const char *name=0 );
    ~CodeGenTabCpp();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagListBox *genBox;

  };


  class OptimizationTabCpp : public QWidget{
  public:
    OptimizationTabCpp(QWidget *parent=0, const char *name=0 );
    ~OptimizationTabCpp();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    QRadioButton *Odefault, *O0, *O1, *O2;
    FlagListBox *optBox;
  };


  class Warnings1TabCpp : public QWidget{
  public:
    Warnings1TabCpp(QWidget *parent=0, const char *name=0 );
    ~Warnings1TabCpp();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagCheckBoxController *controller;
    FlagListBox *wallBox;
  };


  class Warnings2TabCpp : public QWidget{
  public:
    Warnings2TabCpp(QWidget *parent=0, const char *name=0 );
    ~Warnings2TabCpp();

    void readFlags(QStringList *str);
    void writeFlags(QStringList *str);

  private:
    FlagListBox *wrestBox;
  };

  class CppOptionsDlg : public KDialogBase{
    Q_OBJECT

  public:
    CppOptionsDlg(QWidget *parent=0, const char *name=0 );
    ~CppOptionsDlg();
    QString flags() const;
    QString setFlags(const QString &flags);

  private:
    GeneralTabCpp *general;
    CodeGenTabCpp *codegen;
    OptimizationTabCpp *optimization;
    Warnings1TabCpp *warnings1;
    Warnings2TabCpp *warnings2;
  };
#endif
