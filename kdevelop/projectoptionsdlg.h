/***************************************************************************
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PROJECTOPTIONSDLG_H_
#define _PROJECTOPTIONSDLG_H_

#include <kdialogbase.h>
#include <klineedit.h>

class KDevCompilerOptions;
class ServiceComboBox;


class GeneralPage : public QWidget
{
    Q_OBJECT
    
public:
    GeneralPage( QWidget *parent=0, const char *name=0 );
    ~GeneralPage();
};


class CompilerPage : public QWidget
{
    Q_OBJECT
    
public:
    CompilerPage( QWidget *parent=0, const char *name=0 );
    ~CompilerPage();

    QString cCompiler() const;
    QString cxxCompiler() const;
    QString f77Compiler() const;
    QString cFlags() const;
    QString cxxFlags() const;
    QString f77Flags() const;
    void setCFlags(const QString &str);
    void setCxxFlags(const QString &str);
    void setF77Flags(const QString &str);

private slots:
    void cflagsClicked();
    void cxxflagsClicked();
    void f77flagsClicked();

private:
    KDevCompilerOptions *createCompilerOptions(const QString &lang);

    ServiceComboBox *cservice_combo;
    ServiceComboBox *cxxservice_combo;
    ServiceComboBox *f77service_combo;
    QLineEdit *cbinary_edit;
    QLineEdit *cxxbinary_edit;
    QLineEdit *f77binary_edit;
    QLineEdit *cflags_edit;
    QLineEdit *cxxflags_edit;
    QLineEdit *f77flags_edit;
};


class ProjectOptionsDialog : public KDialogBase
{
    Q_OBJECT
    
public:
    ProjectOptionsDialog( QWidget *parent=0, const char *name=0 );
    ~ProjectOptionsDialog();

protected:
    void accept();
    
private:
    GeneralPage *generalpage;
    CompilerPage *compilerpage;
};

#endif
