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

#ifndef _GCCOPTIONSPLUGIN_H_
#define _GCCOPTIONSPLUGIN_H_

#include <kdialogbase.h>

#include "kdevcompileroptions.h"


class GeneralTab;
class OptimizationTab;
class G77Tab;
class Warnings1Tab;
class Warnings2Tab;


class GccOptionsPlugin : public KDevCompilerOptions
{
    Q_OBJECT

public:
    enum Type { GCC, GPP, G77, Unknown };
    GccOptionsPlugin( QObject *parent, const char *name, const QStringList &args );
    ~GccOptionsPlugin();

    static QString captionForType(Type type);
    
    virtual QString exec(QWidget *parent, const QString &flags);

private:
    Type gcctype;
};


class GccOptionsDialog : public KDialogBase
{
public:
    GccOptionsDialog( GccOptionsPlugin::Type type, QWidget *parent=0, const char *name=0 );
    ~GccOptionsDialog();

    void setFlags(const QString &flags);
    QString flags() const;

private:
    GeneralTab *general;
    OptimizationTab *optimization;
    G77Tab *g77;
    Warnings1Tab *warnings1;
    Warnings2Tab *warnings2;
    QStringList unrecognizedFlags;
};

#endif
