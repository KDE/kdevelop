/***************************************************************************
                          breakpointdialog.h  -  description
                             -------------------
    begin                : Mon Sep 20 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef BREAKPOINTDIALOG_H
#define BREAKPOINTDIALOG_H

#include <klineedit.h>
#include <knuminput.h>
#include <kdialog.h>

#include <qcheckbox.h>

namespace JAVADebugger
{

class Breakpoint;

/**
  *@author John Birch
  */

class BPDialog : public KDialog
{
    Q_OBJECT

public:
    BPDialog( Breakpoint *breakpoint, QWidget *parent=0, const char *name=0 );
    ~BPDialog();

    QString getConditional()      { return conditional_->text(); }
    int getIgnoreCount()          { return ignoreCount_->value(); }
    bool isEnabled()              { return enabled_->isChecked(); }

private:
    QCheckBox *enabled_;
    KLineEdit *conditional_;
    KIntNumInput *ignoreCount_;
};

}

#endif
