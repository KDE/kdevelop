/***************************************************************************
 *   Copyright (C) 1998 by Sandy Meier                                     *
 *   smeier@rz.uni-potsdam.de                                              *
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CPPNEWCLASSDLG_H_
#define _CPPNEWCLASSDLG_H_

#include <qlineedit.h>
#include "cppnewclassdlgbase.h"

class KDevPlugin;
class KDevProject;

class CppNewClassDialog : public CppNewClassDialogBase
{
    Q_OBJECT

public:
    CppNewClassDialog(KDevPlugin *part, QWidget *parent=0, const char *name=0);
    ~CppNewClassDialog();

protected:
    virtual void accept();
    virtual void classNameChanged(const QString &text);
    virtual void headerChanged();
    virtual void implementationChanged();
    virtual void nameHandlerChanged(const QString &text);

private:

    bool headerModified;
    bool implementationModified;
    QString m_parse;
    KDevPlugin *m_part;

    // configuration variables
    QString interface_url;
    QString implementation_url;
    QString interface_suffix;
    QString implementation_suffix;
    bool lowercase_filenames;

    friend class ClassGenerator;

    // The class that translates UI input to a class
    class ClassGenerator {

    public:
      ClassGenerator(CppNewClassDialog& _dlg) : dlg(_dlg) {}
      void generate();

    private:
      bool validateInput();
      void common_text();
      void gen_implementation();
      void gen_interface();

      QString className;
      QString header;
      QString implementation;

      KDevProject *project;
      QString subDir, headerPath, implementationPath;
      QString doc;
      QString namespaceStr, baseName;
      bool childClass;
      bool objc;
      QString namespaceBeg, namespaceEnd;
      QString args;

      CppNewClassDialog& dlg;
    };
};

#endif
