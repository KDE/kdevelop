/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GCCOPTIONS_H_
#define _GCCOPTIONS_H_

#include <qlist.h>
#include <kdialogbase.h>

#include "kdevcompileroptions.h"


class GeneralTab;
class CodeGenTab;
class OptimizationTab;
class Warnings1Tab;
class Warnings2Tab;


class GccOptionsPlugin : public KDevCompilerOptions
{
    Q_OBJECT

public:
    GccOptionsPlugin( bool cpp, QObject *parent=0, const char *name=0 );
    ~GccOptionsPlugin();

    virtual QString exec(QWidget *parent, const QString &flags);

private:
    bool withcpp;
};


class GccOptionsDialog : public KDialogBase
{
public:
    GccOptionsDialog(bool cpp, QWidget *parent=0, const char *name=0 );
    ~GccOptionsDialog();

    void setFlags(const QString &flags);
    QString flags() const;

private:
    GeneralTab *general;
    CodeGenTab *codegen;
    OptimizationTab *optimization;
    Warnings1Tab *warnings1;
    Warnings2Tab *warnings2;
};

#endif
