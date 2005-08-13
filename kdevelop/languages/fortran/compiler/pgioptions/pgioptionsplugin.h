/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PGIOPTIONSPLUGIN_H_
#define _PGIOPTIONSPLUGIN_H_

#include <kdialogbase.h>

#include "kdevcompileroptions.h"

class GeneralTab;
class OptimizationTab;
class HpfTab;


class PgiOptionsPlugin : public KDevCompilerOptions
{
    Q_OBJECT

public:
    enum Type { PGHPF, PGF77 };
    
    PgiOptionsPlugin( Type type, QObject *parent=0, const char *name=0 );
    ~PgiOptionsPlugin();

    virtual QString exec(QWidget *parent, const QString &flags);

private:
    Type pgitype;
};


class PgiOptionsDialog : public KDialogBase
{
public:
    PgiOptionsDialog(PgiOptionsPlugin::Type type, QWidget *parent=0, const char *name=0 );
    ~PgiOptionsDialog();

    void setFlags(const QString &flags);
    QString flags() const;

private:
    OptimizationTab *optimization;
    HpfTab *hpf;
};

#endif
