#include <sys/stat.h>
#include <sys/types.h>


#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qfile.h>
#include <qfileinfo.h>


#include <klineedit.h>
#include <kcombobox.h>
#include <keditlistbox.h>
#include <kdebug.h>


#include "addclassdlg.h"


#include "addclass.h"


AddClassInfo::AddClassInfo()
  : interfaceOpt(false), abstractOpt(false), finalOpt(false),
    createConstructor(true), createMain(false)
{
}


QString AddClassInfo::adaFileName() const
{
  QString dest = className;
  dest.replace(QRegExp("\\."), "/");
  return sourceDir + "/" + dest + ".ada";
}


AddClass::AddClass()
{
}


void AddClass::setInfo(const AddClassInfo &info)
{
  m_info = info;
}


AddClassInfo &AddClass::info()
{
  return m_info;
}


void AddClass::setBaseClasses(const QStringList &classes)
{
  m_baseClasses = classes;
}


bool AddClass::showDialog()
{
  AddClassDlg dlg;

  QString dir = m_info.projectDir;
  if (m_info.sourceDir.isEmpty())
    m_info.sourceDir = "src";
  if (dir.isEmpty())
    dir = m_info.sourceDir;
  else
    dir = dir + "/" + m_info.sourceDir;

  dlg.SourcePath->setText(dir);
  dlg.ClassName->setText(m_info.className);
  dlg.Extends->insertStringList(m_baseClasses);
  dlg.Extends->setEditText(m_info.extends);
  dlg.Interface->setChecked(m_info.interfaceOpt);
  dlg.Abstract->setChecked(m_info.abstractOpt);
  dlg.Final->setChecked(m_info.finalOpt);
 
  switch (m_info.visibility)
  {
  case AddClassInfo::ProtectedClass:
    dlg.Protected->setChecked(true);
    break;
  case AddClassInfo::PrivateClass:
    dlg.Private->setChecked(true);
    break;
  default:
    dlg.Public->setChecked(true);
    break;
  }
  
  dlg.Implements->insertStringList(m_info.implements);
  dlg.Constructor->setChecked(m_info.createConstructor);
  dlg.Main->setChecked(m_info.createMain);
  dlg.Documentation->setText(m_info.documentation);
  dlg.License->setEditText(m_info.license);

  if (dlg.exec() == QDialog::Accepted)
  {
    m_info.projectDir = "";
    m_info.sourceDir = dlg.SourcePath->text();
    m_info.className = dlg.ClassName->text();
    m_info.extends = dlg.Extends->currentText();
    m_info.interfaceOpt = dlg.Interface->isChecked();
    m_info.abstractOpt = dlg.Abstract->isChecked();
    m_info.finalOpt = dlg.Final->isChecked();

    if (dlg.Protected->isChecked())
      m_info.visibility = AddClassInfo::ProtectedClass;
    else if (dlg.Private->isChecked())
      m_info.visibility = AddClassInfo::PrivateClass;
    else
      m_info.visibility = AddClassInfo::PublicClass;

    m_info.implements = dlg.Implements->items();
    m_info.createConstructor = dlg.Constructor->isChecked();
    m_info.createMain = dlg.Main->isChecked();
    m_info.documentation = dlg.Documentation->text();
    m_info.license = dlg.License->currentText();

    return true;
  }

  return false;
}


static bool makeDirs(const QString &dest)
{
  QStringList dirs = QStringList::split("/", dest);

  QString d = "";

  for (QStringList::Iterator it=dirs.begin(); it != dirs.end(); ++it)
  {
    d = d + "/" + *it;

    QFileInfo fi(d);

    if (fi.exists() && !fi.isDir())
    {
      /// @todo message to user!
      return false;
    }
    
    if (!fi.exists())
      if (::mkdir(QFile::encodeName(d), 0755) != 0)
        return false;
  }

  return true;
}


bool AddClass::generate()
{
  QString code;

  // license

  if (!m_info.license.isEmpty())
  {
    code += "/*\n";
    
    if (m_info.license == "GPL")
    {
      code +=
        " * This program is free software; you can redistribute it and/or modify\n"
        " * it under the terms of the GNU General Public License as published by\n"
        " * the Free Software Foundation; either version 2 of the License, or\n"
        " * (at your option) any later version.\n";
    }
    else if (m_info.license == "LGPL")
    {
      code +=
	 " * This program is free software; you can redistribute it and/or modify\n"
	 " * it under the terms of the GNU Library General Public License as\n"
	 " * published by the Free Software Foundation; either version 2 of the\n"
	 " * License, or (at your option) any later version.\n";
    }
    else if (m_info.license == "QPL")
    {
      code += 
	" * This program may be distributed under the terms of the Q Public\n"
	" * License as defined by Trolltech AS of Norway and appearing in the\n"
        " * file LICENSE.QPL included in the packaging of this file.\n"
        " *\n"
        " * This program is distributed in the hope that it will be useful,\n"
        " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
    }
    else
    {
      code += " * This program is licensed under the " + m_info.license + ".\n";
      code += " * Please see the documentation for details.\n";
    }	    

    code += " */\n\n\n";
  }
  
  // find class and package name
  QString className, packageName;

  int i = m_info.className.findRev('.');
  if (i == -1)
  {
    packageName = "";
    className = m_info.className;
  }
  else
  {
    packageName = m_info.className.left(i);
    className = m_info.className.mid(i+1);
  }

  // package
  if (!packageName.isEmpty())
    code += "package " + packageName + ";\n\n\n";
  
  // documentation

  if (!m_info.documentation.isEmpty())
  {
    code += "/**\n";

    QTextStream ts(&m_info.documentation, IO_ReadOnly);

    while (!ts.eof())
      code += " * " + ts.readLine() + "\n";
    
    code += " */\n\n";
  }
  
  // visibility
  
  switch (m_info.visibility)
  {
  case AddClassInfo::PrivateClass:
    code += "private";
    break;
  case AddClassInfo::ProtectedClass:
    code += "protected";
    break;
  default:
    code += "public";
    break;
  }
  
  // abstract, final
  
  if (!m_info.interfaceOpt)
  {
    if (m_info.abstractOpt)
      code += " abstract";
    if (m_info.finalOpt)
      code += " final";
  }
  
  // generate class/interface declaration

  if (m_info.interfaceOpt)
    code += " interface ";
  else
    code += " class ";

  // classname
  
  code += className;

  // extends
  
  if (!m_info.extends.isEmpty())
    code += " extends " + m_info.extends;

  // implements
  
  if ((m_info.implements.count() > 0) && !m_info.interfaceOpt)
  {
    code += " implements ";
    unsigned int c=0;
    for (QStringList::Iterator it = m_info.implements.begin(); it != m_info.implements.end(); ++it, c++)
    {
      code += *it;
      if (c+1 < m_info.implements.count())
        code += ", ";
    }
  }
	  
  // body start

  code += "\n{\n\n";

  // default constructor

  if (m_info.createConstructor && !m_info.interfaceOpt)
  {
    code += "  " + className + "()\n";
    code += "  {\n";
    if (!m_info.extends.isEmpty())
      code += "    super();\n";
    code += "  }\n\n";
  }

  // main method

  if (m_info.createMain && !m_info.interfaceOpt)
  {
    code += "  public static void main(String[] args)\n";
    code += "  {\n";
    code += "  }\n\n";
  }

  // body end

  code += "};\n";

  // create directories

  QString dest = packageName;
  dest.replace(QRegExp("\\."), "/");
  dest = m_info.sourceDir + "/" + dest;

  if (!makeDirs(dest))
    return false;

  // write out the file

  if (QFile::exists(m_info.adaFileName()))
  {
    /// @todo ask before overwriting!
  }
  
  QFile of(m_info.adaFileName());
  if (!of.open(IO_WriteOnly))
  {
    /// @todo message to user
    return false;
  }

  QTextStream os(&of);
  os << code;

  of.close();

  return true;
}
