/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DCCOPTIONSPLUGIN_H
#define DCCOPTIONSPLUGIN_H

#include <kdialogbase.h>

#include "kdevcompileroptions.h"

class DccOptionsPlugin : public KDevCompilerOptions
{
    Q_OBJECT

public:
    DccOptionsPlugin( QObject *parent, const char *name, const QStringList/* &args */);
    ~DccOptionsPlugin();

    virtual QString exec(QWidget *parent, const QString &flags);
};

class GeneralTab;
class LinkerTab;
class LocationsTab;
class Locations2Tab;
class CodegenTab;
class DebugOptimTab;

class DccOptionsDialog : public KDialogBase
{
public:
    DccOptionsDialog( QWidget *parent=0, const char *name=0 );
    ~DccOptionsDialog();

    void setFlags(const QString &flags);
    QString flags() const;

private:
    GeneralTab *general;
    LinkerTab *linker;
    LocationsTab *locations;
    Locations2Tab *locations2;
    CodegenTab *codegen;
    DebugOptimTab *debug_optim;
    QStringList unrecognizedFlags;
};


#endif
