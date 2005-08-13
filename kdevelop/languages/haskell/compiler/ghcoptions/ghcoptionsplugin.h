/***************************************************************************
                          ghcoptionsplugin.h  -  description
                             -------------------
    begin                : Sat Aug 16 2003
    copyright            : (C) 2003 by Peter Robinson
    email                : listener@thaldyron.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GHCOPTIONSPLUGIN_H
#define GHCOPTIONSPLUGIN_H

#include "kdialogbase.h"
#include "kdevcompileroptions.h"



class GhcOptionsPlugin : public KDevCompilerOptions
{
   Q_OBJECT
   
public:
  	GhcOptionsPlugin( QObject *parent, const char *name, const QStringList &args );
  	~GhcOptionsPlugin();

  	virtual QString exec( QWidget *parent, const QString &flags );
};


/*class FeedbackTab;
class FilesAndDirectoriesTab;
class LanguageTab;
class CodegenTab;*/

class GhcOptionsDialog : public KDialogBase
{
public:
    GhcOptionsDialog( QWidget *parent=0, const char *name=0 );
    ~GhcOptionsDialog();

    void setFlags(const QString &flags);
    QString flags() const;

private:
/*    GeneralTab *general;
    OptimizationTab *optimization;
    G77Tab *g77;
    Warnings1Tab *warnings1;
    Warnings2Tab *warnings2;*/

/*    FeedbackTab *feedback;
    FilesAndDirectoriesTab *directories;
    FilesAndDirectoriesTab2 *directories2;
    DebugOptimTab *debug_optim;
    LanguageTab *language;
    CodegenTab *codegen;
    AssemblerTab *assembler;
    LinkerTab *linker;
    MiscTab *misc;
    QStringList unrecognizedFlags;*/
};

#endif
