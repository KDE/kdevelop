/***************************************************************************
 *   Copyright (C) 1998 by Sandy Meier                                     *
 *   smeier@rz.uni-potsdam.de                                              *
 *   Copyright (C) 1999 by Benoit.Cerrina                                  *
 *   Benoit.Cerrina@writeme.com                                            *
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cppnewclassdlg.h"

#include <qcheckbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qtextedit.h>
#include <klocale.h>
#include <kmessagebox.h>


CppNewClassDialog::CppNewClassDialog(QWidget *parent, const char *name)
    : CppNewClassDialogBase(parent, name)
{
    headerModified = false;
    implementationModified = false;
}


CppNewClassDialog::~CppNewClassDialog()
{}


void CppNewClassDialog::classNameChanged(const QString &text)
{
    QString str = text;
    
    if (!headerModified)
        header_edit->setText(str + ".h");

    if (!implementationModified)
        implementation_edit->setText(str + ".cpp");
}


void CppNewClassDialog::headerChanged()
{
    // Only if a change caused by the user himself
    if (header_edit->hasFocus())
        headerModified = true;
}


void CppNewClassDialog::implementationChanged()
{
    // Only if a change caused by the user himself
    if (implementation_edit->hasFocus())
        implementationModified = true;
}


void CppNewClassDialog::accept()
{
    QString className = classname_edit->text();
    if (className.isEmpty()) {
        KMessageBox::error(this, i18n("You must enter a classname."));
        return;
    }
    
    QString header = header_edit->text(); 
    if (header.isEmpty()) {
        KMessageBox::error(this, i18n("You must enter a name for the header file."));
        return;
    }
    QString implementation = implementation_edit->text(); 
    if (implementation.isEmpty() ){
        KMessageBox::error(this,i18n("You must enter a name for the implementation file."));
        return;
    }

    if (header.find('/') != -1 || implementation.find('/') != -1) {
        KMessageBox::error(this, i18n("Generated files will always be added to the "
                                      "active directory, so you must not give an "
                                      "explicit sub directory."));
        return;
    }
    
    // header.prepend(subDir);
    // implementation.prepend(subDir);
    
    if (QFileInfo(header).exists() || QFileInfo(implementation).exists()) {
        KMessageBox::error(this, i18n("Sorry, but KDevelop is not able to add classes "
                                      "to existing header or implementation files."));
        return;
    }
    
    QString baseName = basename_edit->text();
    if (baseName.isEmpty() && childclass_box->isChecked())
        baseName = "QWidget";
    
    QString doc = documentation_edit->text();
    
    if (filetemplate_box->isChecked()) {
        // Copy file templates
    }

    QStringList ilist;

    // Read in the existing file
    QFile ifile(implementation);
    QTextStream istream(&ifile);
    if (ifile.open(IO_ReadOnly)) {
        while(!istream.eof())
            ilist << istream.readLine();
    }
    ifile.close();
    
    //Added by Benoit Cerrina, Benoit.Cerrina@writeme.com
    //computes the relative position of the header and implementation
    //files in order for the #include statement to work correctly.
    //without this the generated file may not compile
    // 6 Dec 99
    QString relPath;
									
    for (int i = implementation.findRev('/'); i != -1; i = implementation.findRev('/', --i))
    	relPath += "../";

    bool objc = false;
    if (objc) {
        ilist << "\n#include \"" + relPath + header + "\"\n";
        ilist << "@implementation " + className;
        ilist << "@end\n";
    } else {
        ilist << "\n#include \"" + relPath + header + "\"\n";

        // Constructor
        QString cons = className + "::" + className + "(" ;
        if (childclass_box->isChecked()) {
            cons += "QWidget *parent, const char *name ) : ";
            cons += baseName;
            cons += "(parent, name";
        }
        cons += ")";
        ilist << cons;
        ilist << "{";

        // Destructor
        ilist << className + "::~" + className +"(){";
        ilist << "{";
        ilist << "}";
    }

    if (!ifile.open(IO_WriteOnly)) {
        KMessageBox::error(this, "Cannot write to implementation file");
        return;
    }
    
    QStringList::ConstIterator iit;
    for (iit = ilist.begin(); iit != ilist.end(); ++iit)
        istream << (*iit) << "\n";
    ifile.close();

    QStringList hlist;

    // Read in the existing file
    QFile hfile(header);
    QTextStream hstream(&hfile);
    if (hfile.open(IO_ReadOnly)) {
        while(!hstream.eof())
            hlist << hstream.readLine();
    }
    hfile.close();

    QString inclusionGuard = header;
    inclusionGuard.replace(QRegExp("\\."),"_");
    hlist << "\n#ifndef _" + header.upper() + "_";
    hlist << "#define "+ header.upper() + "\n";
    
    if (childclass_box->isChecked())
        hlist << "#include <qwidget.h>";
    if (objc)
        hlist << "#include <Foundation/NSObject.h>";
    
    if (!baseName.isEmpty()){
        if (objc) {
            if (baseName != "NSObject") {
                hlist << "#include \"" + baseName + ".h\"";
            }
        } else {
            hlist << "#include <" + baseName.lower() + ".h>\n";
        }
    }

    // FIXME: author
    QString author;
    
    hlist << "\n/**" + doc + "\n";
    hlist << "  *@author "+ author;
    hlist << "  */\n";
    if (objc) {
        QString classDecl = "@interface " + className + " : ";
        if (baseName.isEmpty())
            classDecl += "NSObject";
        else
            classDecl += baseName;
        hlist << classDecl;
        hlist << "@end ";
        hlist << "\n#endif";
    } else {
        QString classDecl = "class " + className;
        if (!baseName.isEmpty()) {
            classDecl += " : ";
            if (virtual_box->isChecked())
                classDecl += "virtual ";
            if (public_button->isChecked())
                classDecl += "public ";
            if (protected_button->isChecked())
                classDecl += "protected ";
            if (private_button->isChecked())
                classDecl += "private ";
            classDecl += baseName;
        }
        hlist << classDecl;
        hlist << "{";
        if (childclass_box->isChecked())
            hlist << "  Q_OBJECT\n";
        hlist << "public: \n";

        // Constructor
        QString cons = "  " + className + "(";
        if (childclass_box->isChecked())
            cons += "QWidget *parent=0, const char *name=0";
        cons += ");";
        hlist << cons;

        // Destructor
        hlist << "  ~" + className +"();";
        hlist << "};\n\n#endif";
    }

    if (!hfile.open(IO_WriteOnly)) {
        KMessageBox::error(this, "Cannot write to heade file");
        return;
    }
    
    QStringList::ConstIterator hit;
    for (hit = hlist.begin(); hit != hlist.end(); ++hit)
        hstream << (*hit) << "\n";
    hfile.close();
}
