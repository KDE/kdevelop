/***************************************************************************
*   Copyright (C) 1998 by Sandy Meier                                     *
*   smeier@rz.uni-potsdam.de                                              *
*   Copyright (C) 1999 by Benoit.Cerrina                                  *
*   Benoit.Cerrina@writeme.com                                            *
*   Copyright (C) 2002 by Bernd Gehrmann                                  *
*   bernd@kdevelop.org                                                    *
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
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
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qcombobox.h>
#include <qlistview.h>


#include "kdevplugin.h"
#include "kdevproject.h"
#include "domutil.h"
#include "filetemplate.h"


CppNewClassDialog::CppNewClassDialog(KDevPlugin *part, QWidget *parent, const char *name)
	: CppNewClassDialogBase(parent, name)
{
    headerModified = false;
    implementationModified = false;
    m_part = part;
    // read file template configuration
    //    KDevProject *project = part->project();
    QDomDocument &dom = *part->projectDom();
    interface_url = DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/interfaceURL");
    implementation_url = DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/implementationURL");
    interface_suffix = DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/interfacesuffix", ".h");
    implementation_suffix = DomUtil::readEntry(dom, "/cppsupportpart/filetemplates/implementationsuffix", ".cpp");
    lowercase_filenames = DomUtil::readBoolEntry(dom, "/cppsupportpart/filetemplates/lowercasefilenames", true);
    m_parse = DomUtil::readEntry( *m_part->projectDom(), "/cppsupportpart/newclass/filenamesetting","none");
    //    name_handler_combo->setCurrentText(m_parse);
    baseclasses_view->setSorting(-1);
}


CppNewClassDialog::~CppNewClassDialog()
{}


void CppNewClassDialog::nameHandlerChanged(const QString &text)
{
	DomUtil::writeEntry( *m_part->projectDom(), "/cppsupportpart/newclass/filenamesetting",text);
	m_parse = text;
	classNameChanged(classname_edit->text());
}

void CppNewClassDialog::classNameChanged(const QString &text)
{
    QString str = text;
    
    if (!headerModified) {
        QString header = str + interface_suffix;
        if (lowercase_filenames)
            header = header.lower();
        header_edit->setText(header);
    }
    if (!implementationModified) {
        QString implementation = str + implementation_suffix;
        if (lowercase_filenames)
            implementation = implementation.lower();
        implementation_edit->setText(implementation);
    }
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

void CppNewClassDialog::checkObjCInheritance(int val)
{
    if (val && (baseclasses_view->childCount() > 1))
        if (KMessageBox::warningContinueCancel(this, 
        i18n("Objective C does not support multiple inheritance.\nOnly the first base class in the list will be taken into account"),
        i18n("Warning"), KStdGuiItem::cont(), "Check Objective C inheritance rules" ) == KMessageBox::Cancel)
        objc_box->setChecked(false);
}

void CppNewClassDialog::checkQWidgetInheritance(int val)
{
    if (val && (baseclasses_view->childCount() > 1))
        if (KMessageBox::warningContinueCancel(this, 
            i18n("Multiple inheritance requires QObject derivative to be first and unique in base class list."),
            i18n("Warning"), KStdGuiItem::cont(), "Check QWidget inheritance rules" ) == KMessageBox::Cancel)
            childclass_box->setChecked(false);
}

void CppNewClassDialog::accept()
{
  ClassGenerator generator(*this);
  generator.generate();

  QDialog::accept();

}

void CppNewClassDialog::setStateOfInheritanceEditors(bool state)
{
    basename_edit->setEnabled(state);
    virtual_box->setEnabled(state);
    public_button->setEnabled(state);
    protected_button->setEnabled(state);
    private_button->setEnabled(state);
    baseclasses_view->setEnabled(state);
}

void CppNewClassDialog::addBaseClass()
{
    if (baseclasses_view->selectedItem())
        baseclasses_view->selectedItem()->setSelected(false);
    QListViewItem* it = new QListViewItem(baseclasses_view, baseclasses_view->lastItem(), QString::null, "public");
    setStateOfInheritanceEditors(true);
    public_button->setChecked(true);
    virtual_box->setChecked(false);
    basename_edit->setText(QString::null);
    basename_edit->setFocus();
    baseclasses_view->setSelected(it, true);
}
    
void CppNewClassDialog::remBaseClass()
{
    if (baseclasses_view->selectedItem())
    {
	QListViewItem *it = baseclasses_view->selectedItem();
	baseclasses_view->selectedItem()->setSelected(false);
        delete it;
        if (baseclasses_view->childCount() == 0)
            setStateOfInheritanceEditors(false);
    }
}

void CppNewClassDialog::currBaseNameChanged(const QString &text)
{
    if ( baseclasses_view->selectedItem() )
    {
        baseclasses_view->selectedItem()->setText(0, text);
    }
}

void CppNewClassDialog::currBasePrivateSet()
{
    if ( baseclasses_view->selectedItem() )
    {
        baseclasses_view->selectedItem()->setText(1, (virtual_box->isChecked()?"virtual ":"") + QString("private"));
    }    
}

void CppNewClassDialog::currBaseProtectedSet()
{
    if ( baseclasses_view->selectedItem() )
    {
        baseclasses_view->selectedItem()->setText(1, (virtual_box->isChecked()?"virtual ":"") + QString("protected"));
    }    
}

void CppNewClassDialog::currBasePublicSet()
{
    if ( baseclasses_view->selectedItem() )
    {
        baseclasses_view->selectedItem()->setText(1, (virtual_box->isChecked()?"virtual ":"") + QString("public"));
    }    
}

void CppNewClassDialog::currBaseVirtualChanged(int val)
{
    if ( baseclasses_view->selectedItem() )
    {
        baseclasses_view->selectedItem()->setText(1, QString(val?"virtual ":"") + 
         QString(private_button->isChecked()?"private":"") +
         QString(protected_button->isChecked()?"protected":"") +
         QString(public_button->isChecked()?"public":""));
    }
}

void CppNewClassDialog::currBaseSelected(QListViewItem *it)
{
    if (it == 0) return;
    basename_edit->setText(it->text(0));
    if (it->text(1).contains("private"))
        private_button->setChecked(true);
    else
        private_button->setChecked(false);
    if (it->text(1).contains("protected"))
        protected_button->setChecked(true);
    else
        protected_button->setChecked(false);
    if (it->text(1).contains("public"))
        public_button->setChecked(true);
    else
        public_button->setChecked(false);
    if (it->text(1).contains("virtual"))
        virtual_box->setChecked(true);
    else
        virtual_box->setChecked(false);
}

bool CppNewClassDialog::ClassGenerator::validateInput()
{
  className = dlg.classname_edit->text().simplifyWhiteSpace();
  if (className.isEmpty()) {
    KMessageBox::error(&dlg, i18n("You must enter a classname."));
    return false;
  }
    
  header = dlg.header_edit->text().simplifyWhiteSpace(); 
  if (header.isEmpty()) {
    KMessageBox::error(&dlg, i18n("You must enter a name for the header file."));
    return false;
  }
  implementation = dlg.implementation_edit->text().simplifyWhiteSpace(); 
  if (implementation.isEmpty() ){
    KMessageBox::error(&dlg, i18n("You must enter a name for the implementation file."));
    return false;
  }

  // FIXME
  if (header.find('/') != -1 || implementation.find('/') != -1) {
    KMessageBox::error(&dlg, i18n("Generated files will always be added to the "
				  "active directory, so you must not give an "
				  "explicit subdirectory."));
    return false;
  }

  return true;
}


void CppNewClassDialog::ClassGenerator::generate()
{
  if (!validateInput())
    return;
   
  common_text();

  gen_implementation();

  gen_interface();

}

void CppNewClassDialog::ClassGenerator::common_text()
{

  // common

  project = dlg.m_part->project();
  subDir = project->projectDirectory() + "/";
  if (!project->activeDirectory().isEmpty())
    subDir += project->activeDirectory() + "/";
  headerPath = subDir + header;
  implementationPath = subDir + implementation;
    
  if (QFileInfo(headerPath).exists() || QFileInfo(implementationPath).exists()) {
    KMessageBox::error(&dlg, i18n("KDevelop is not able to add classes "
				  "to existing header or implementation files."));
    return;
  }
    
  namespaceStr = dlg.namespace_edit->text(); 
  childClass = dlg.childclass_box->isChecked();
  objc = dlg.objc_box->isChecked();

  if ( (dlg.baseclasses_view->childCount() == 0) && childClass)
    QListViewItem* it = new QListViewItem(dlg.baseclasses_view, "QWidget", "public");
  if (objc && (dlg.baseclasses_view->childCount() == 0))
    QListViewItem* it = new QListViewItem(dlg.baseclasses_view, "NSObject", "public");
    
  doc = dlg.documentation_edit->text();
    
  if (!namespaceStr.isEmpty()) {
    namespaceBeg = "namespace " + namespaceStr + " {\n";
    namespaceEnd = "}\n";
  }
}

void CppNewClassDialog::ClassGenerator::gen_implementation()
{

  // implementation
        
  QString classImpl;
  if (dlg.filetemplate_box->isChecked()) {
    QDomDocument dom = *dlg.m_part->projectDom();
    if(DomUtil::readBoolEntry(dom,"/cppsupportpart/filetemplates/choosefiles",false))
      classImpl = FileTemplate::read(dlg.m_part, DomUtil::readEntry(dom,"/cppsupportpart/filetemplates/implementationURL",""), FileTemplate::Custom);
    else
      classImpl = FileTemplate::read(dlg.m_part, "cpp");
  }
  if (objc) {
    classImpl += QString("\n"
			 "#include \"$HEADER$\"\n"
			 "@implementation $CLASSNAME$\n"
			 "@end\n");
  } else {
    classImpl += QString("// $FILENAME\n"
			 "#include \"$HEADER$\"\n"
			 "\n"
			 "\n")
      + namespaceBeg
      + QString("$CLASSNAME$::$CLASSNAME$($ARGS$)\n"
		"$BASEINITIALIZER$\n"
		"{\n"
		"}\n"
		"\n"
		"\n"
		"$CLASSNAME$::~$CLASSNAME$()\n"
		"{\n"
		"}\n")
      + namespaceEnd;
  }

  QString relPath;
  for (int i = implementation.findRev('/'); i != -1; i = implementation.findRev('/', --i))
    relPath += "../";
  args = childClass? "QWidget *parent, const char *name" : "";
  QString baseInitializer;
  
  if (childClass && (dlg.baseclasses_view->childCount() == 0))
    baseInitializer = "  : QWidget(parent, name)";
  else if (dlg.baseclasses_view->childCount() != 0)
  {
    QListViewItemIterator it( dlg.baseclasses_view );
    while ( it.current() )
    {
      if (!it.current()->text(0).isNull())
      {
        baseInitializer += " : ";
        if (childClass && (baseInitializer == " : ")) 
          baseInitializer += it.current()->text(0) + "(parent, name)";
        else
          baseInitializer += it.current()->text(0) + "()";
      }       
      ++it;
    }
  } 
   
  classImpl.replace(QRegExp("\\$HEADER\\$"), relPath+header);
  classImpl.replace(QRegExp("\\$BASEINITIALIZER\\$"), baseInitializer);
  classImpl.replace(QRegExp("\\$CLASSNAME\\$"), className);
  classImpl.replace(QRegExp("\\$ARGS\\$"), args);
  classImpl.replace(QRegExp("\\$FILENAME\\$"), implementation);
      
  QFile ifile(implementationPath);
  if (!ifile.open(IO_WriteOnly)) {
    KMessageBox::error(&dlg, "Cannot write to implementation file");
    return;
  }
  QTextStream istream(&ifile);
  istream << classImpl;
  ifile.close();

}


void CppNewClassDialog::ClassGenerator::gen_interface()
{
  // interface

  QString classIntf;
  if (dlg.filetemplate_box->isChecked()) {
    QDomDocument dom = *dlg.m_part->projectDom();
    if(DomUtil::readBoolEntry(dom,"/cppsupportpart/filetemplates/choosefiles",false))
      classIntf =
	FileTemplate::read(dlg.m_part, DomUtil::readEntry(dom,"/cppsupportpart/filetemplates/interfaceURL",""), FileTemplate::Custom);
    else
      classIntf  = FileTemplate::read(dlg.m_part, "h");
  }
        
  if (objc) {
    classIntf = QString("\n"
			"#ifndef _$HEADERGUARD$_\n"
			"#define _$HEADERGUARD$_\n"
			"\n"
			"$INCLUDEBASEHEADER$\n"
			"#include <Foundation/NSObject.h>\n"
			"\n"
			"\n"
			"/**\n"
			" * $DOC$\n"
			" * $AUTHOR$\n"
			" **/\n"
			"@interface $CLASSNAME$ : $BASECLASS$\n"
			"@end\n"
			"\n"
			"#endif\n"
			);
  } else {
    classIntf = QString("\n"
			"#ifndef $HEADERGUARD$\n"
			"#define $HEADERGUARD$\n"
			"\n"
			"$INCLUDEBASEHEADER$\n"
			"\n")
      + namespaceBeg 
      + QString("/**\n"
		" * $DOC$\n"
		" * $AUTHOR$\n"
		" **/\n"
		"class $CLASSNAME$$INHERITANCE$\n"
		"{\n"
		"$QOBJECT$\n"
		"public:\n"
		"  $CLASSNAME$($ARGS$);\n"
		"  ~$CLASSNAME$();\n"
		"};\n"
		"\n")
      + namespaceEnd
      +         "#endif\n";
  }
                   
  QString headerGuard = namespaceStr.upper() + header.upper();
  headerGuard.replace(QRegExp("\\."),"_");
  QString includeBaseHeader;
  if (childClass) // TODO: do this only if this is a Qt class
  {
    includeBaseHeader = "#include <qwidget.h>";
  }
  if (objc) {
    if (dlg.baseclasses_view->firstChild())
      if (dlg.baseclasses_view->firstChild()->text(0) != "NSObject")
        includeBaseHeader = "#include \"" + dlg.baseclasses_view->firstChild()->text(0) + ".h\"";
  } else
  {
    QListViewItemIterator it( dlg.baseclasses_view );
    while ( it.current() )
    {
      if (!it.current()->text(0).isEmpty())
        if ((!childClass) || (it.current()->text(0) != "QWidget"))
          includeBaseHeader += "\n#include <" + it.current()->text(0).lower() + ".h>";
      ++it;
    }
  }
    
  QString author = DomUtil::readEntry(*dlg.m_part->projectDom(), "/general/author");
    
  QString inheritance;
  if (dlg.baseclasses_view->childCount() > 0) 
  {
    inheritance += " : ";

    QListViewItemIterator it( dlg.baseclasses_view );
    while ( it.current() )
    {
      if (!it.current()->text(0).isEmpty())
      {
        if (inheritance != " : ")
          inheritance += ", ";
        if (it.current()->text(1).contains("virtual"))
          inheritance += "virtual ";
        if (it.current()->text(1).contains("public"))
          inheritance += "public ";
        if (it.current()->text(1).contains("protected"))
          inheritance += "protected ";
        if (it.current()->text(1).contains("private"))
          inheritance += "private ";
        inheritance += it.current()->text(0);
      }
      ++it;
    }
  }

  QString qobject;
  if (childClass)
    qobject = "  Q_OBJECT\n";

  classIntf.replace(QRegExp("\\$HEADERGUARD\\$"), headerGuard);
  classIntf.replace(QRegExp("\\$INCLUDEBASEHEADER\\$"), includeBaseHeader);
  classIntf.replace(QRegExp("\\$AUTHOR\\$"), author);
  classIntf.replace(QRegExp("\\$DOC\\$"), doc);
  classIntf.replace(QRegExp("\\$CLASSNAME\\$"), className);
  if (dlg.baseclasses_view->childCount() > 0) 
    classIntf.replace(QRegExp("\\$BASECLASS\\$"), dlg.baseclasses_view->firstChild()->text(0));
  classIntf.replace(QRegExp("\\$INHERITANCE\\$"), inheritance);
  classIntf.replace(QRegExp("\\$QOBJECT\\$"), qobject);
  classIntf.replace(QRegExp("\\$ARGS\\$"), args);
  classIntf.replace(QRegExp("\\$FILENAME\\$"), header);

        
  QFile hfile(headerPath);
  if (!hfile.open(IO_WriteOnly)) {
    KMessageBox::error(&dlg, "Cannot write to header file");
    return;
  }
  QTextStream hstream(&hfile);
  hstream << classIntf;
  hfile.close();

	QStringList fileList;

	if ( project->activeDirectory() == QString::null )
	{
		fileList.append ( header );
		fileList.append ( implementation );
	}
	else
	{
		fileList.append ( project->activeDirectory() + "/" + header );
		fileList.append ( project->activeDirectory() + "/" + implementation );
	}

	project->addFiles ( fileList );
}

#include "cppnewclassdlg.moc"
