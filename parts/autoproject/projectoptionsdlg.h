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

#ifndef _PROJECTOPTIONSDLG_H_
#define _PROJECTOPTIONSDLG_H_

#include <qtabdialog.h>

#include "kdevcompileroptions.h"

class QCheckBox;
class QLineEdit;
class QSpinBox;
class ServiceComboBox;
class AutoProjectPart;


class ProjectOptionsDialog : public QTabDialog
{
    Q_OBJECT
    
public:
    ProjectOptionsDialog( AutoProjectPart *part, QWidget *parent=0, const char *name=0 );
    ~ProjectOptionsDialog();

#if 0
    QString cCompiler() const;
    QString cxxCompiler() const;
    QString f77Compiler() const;
    void setCCompiler(const QString &str);
    void setCxxCompiler(const QString &str);
    void setF77Compiler(const QString &str);
    QString cCompilerBinary() const;
    QString cxxCompilerBinary() const;
    QString f77CompilerBinary() const;
    void setCCompilerBinary(const QString &str);
    void setCxxCompilerBinary(const QString &str);
    void setF77CompilerBinary(const QString &str);
    QString cFlags() const;
    QString cxxFlags() const;
    QString f77Flags() const;
    void setCFlags(const QString &str);
    void setCxxFlags(const QString &str);
    void setF77Flags(const QString &str);
#endif

private slots:
    void cflagsClicked();
    void cxxflagsClicked();
    void f77flagsClicked();

private:
    QWidget *createCompilerTab();
    QWidget *createMakeTab();
    KDevCompilerOptions *createCompilerOptions(const QString &lang);

    void init();
    virtual void accept();
    
    ServiceComboBox *cservice_combo;
    ServiceComboBox *cxxservice_combo;
    ServiceComboBox *f77service_combo;
    QLineEdit *cbinary_edit;
    QLineEdit *cxxbinary_edit;
    QLineEdit *f77binary_edit;
    QLineEdit *cflags_edit;
    QLineEdit *cxxflags_edit;
    QLineEdit *f77flags_edit;

    QCheckBox *abort_box;
    QCheckBox *dontact_box;
    QSpinBox *jobs_box;

    AutoProjectPart *m_part;
};

#endif
