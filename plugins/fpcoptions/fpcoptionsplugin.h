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

class FpcOptionsPlugin : public KDevCompilerOptions
{
    Q_OBJECT

public:
    FpcOptionsPlugin( QObject *parent, const char *name, const QStringList &args );
    ~FpcOptionsPlugin();

    virtual QString exec(QWidget *parent, const QString &flags);
};


class FeedbackTab;
class FilesAndDirectoriesTab;
class LanguageTab;
class CodegenTab;

class FpcOptionsDialog : public KDialogBase
{
public:
    FpcOptionsDialog( QWidget *parent=0, const char *name=0 );
    ~FpcOptionsDialog();

    void setFlags(const QString &flags);
    QString flags() const;

private:
/*    GeneralTab *general;
    OptimizationTab *optimization;
    G77Tab *g77;
    Warnings1Tab *warnings1;
    Warnings2Tab *warnings2;*/

    FeedbackTab *feedback;
    FilesAndDirectoriesTab *directories;
    FilesAndDirectoriesTab2 *directories2;
    DebugOptimTab *debug_optim;
    LanguageTab *language;
    CodegenTab *codegen;
    AssemblerTab *assembler;
    LinkerTab *linker;
    MiscTab *misc;
    QStringList unrecognizedFlags;
};

#endif
