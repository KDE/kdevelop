/***************************************************************************
                          LdLinkerOptionsWidget.h  -  description
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
#include <LdLinkerOptionsWidgetBase.h>
#include "flagboxes.h"
#include <qradiobutton.h>
#include <qvaluelist.h>
#include <qtabwidget.h>
#include <kdialogbase.h>
#include "KDevLinker.h"

/**
  *@author Omid Givi
  */

class GeneralTabLd;
class KDETabLd;
class MiscTabLd;

class LdLinkerOptionsWidget : public LdLinkerOptionsWidgetBase{
   Q_OBJECT
public:
	LdLinkerOptionsWidget(QWidget *parent=0, const char *name=0, QWidget *pdlg=0);
	~LdLinkerOptionsWidget();

	void setLinker(KDevLinker*);

public slots:
    void slotFlagsToolButtonClicked();
    void slotClearAllClicked();
    void slotWidgetStarted();
    void slotButtonApplyClicked();
private:
	KDevLinker* m_pKDevLinker;
};

class GeneralTabLd : public QWidget{
public:
  GeneralTabLd(QWidget *parent=0, const char *name=0 );
  ~GeneralTabLd();

  void readFlags(QStringList *str);
  void writeFlags(QStringList *str);

private:
  FlagCheckBoxController *controller;
};

class KDETabLd : public QWidget{
public:
  KDETabLd(QWidget *parent=0, const char *name=0 );
  ~KDETabLd();

  void readFlags(QStringList *str);
  void writeFlags(QStringList *str);

private:
  FlagCheckBoxController *controller;
};


class MiscTabLd : public QWidget{
public:
  MiscTabLd(QWidget *parent=0, const char *name=0 );
  ~MiscTabLd();

  void readFlags(QStringList *str);
  void writeFlags(QStringList *str);

private:
  FlagCheckBoxController *controller;
};

class LdOptionsDlg : public KDialogBase{
  Q_OBJECT
public:
  LdOptionsDlg(QWidget *parent=0, const char *name=0 );
  ~LdOptionsDlg();
  QString flags() const;
  QString setFlags(const QString &flags);

private:
  GeneralTabLd *generaltab;
  KDETabLd *kdetab;
  MiscTabLd *misctab;
};

#endif
