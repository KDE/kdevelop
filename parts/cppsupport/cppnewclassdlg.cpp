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
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qcombobox.h>

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
	m_parse = DomUtil::readEntry( *m_part->projectDom(), "/cppsupportpart/newclass/filenamesetting","none");
	name_handler_combo->setCurrentText(m_parse);
}


CppNewClassDialog::~CppNewClassDialog()
{}


void CppNewClassDialog::nameHandlerChanged(const QString &text)
{
	qDebug("Changing name handler %s", text.latin1());
	DomUtil::writeEntry( *m_part->projectDom(), "/cppsupportpart/newclass/filenamesetting",text);
	m_parse = text;
	classNameChanged(classname_edit->text());
}

void CppNewClassDialog::classNameChanged(const QString &text)
{
	QString str = text;

	if(m_parse=="lower") str = str.lower();
	else if(m_parse=="upper") str = str.upper();

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
	QString className = classname_edit->text().simplifyWhiteSpace();
	if (className.isEmpty()) {
		KMessageBox::error(this, i18n("You must enter a classname."));
		return;
	}

	QString header = header_edit->text().simplifyWhiteSpace();
	if (header.isEmpty()) {
		KMessageBox::error(this, i18n("You must enter a name for the header file."));
		return;
	}
	QString implementation = implementation_edit->text().simplifyWhiteSpace();
	if (implementation.isEmpty() ){
		KMessageBox::error(this,i18n("You must enter a name for the implementation file."));
		return;
	}

	// FIXME
	if (header.find('/') != -1 || implementation.find('/') != -1) {
		KMessageBox::error(this, i18n("Generated files will always be added to the "
									"active directory, so you must not give an "
									"explicit subdirectory."));
		return;
	}

	KDevProject *project = m_part->project();
	QString subDir = project->projectDirectory() + "/";
	if (!project->activeDirectory().isEmpty())
		subDir += project->activeDirectory() + "/";
	QString headerPath = subDir + header;
	QString implementationPath = subDir + implementation;

	if (QFileInfo(headerPath).exists() || QFileInfo(implementationPath).exists()) {
		KMessageBox::error(this, i18n("Unable to add class source code "
									"to %1 or %2!\nOne of those files already exists!\n"
									"Choose a new file name for your class.").arg ( header ).arg (implementation));
		return;
	}

	QString baseName = basename_edit->text();
	bool childClass = childclass_box->isChecked();
	bool objc = false;

	if (baseName.isEmpty() && childClass)
		baseName = "QWidget";
	if (objc && baseName.isEmpty())
		baseName = "NSObject";

	QString doc = documentation_edit->text();

	QString istr;
	if (filetemplate_box->isChecked()) {
	QDomDocument dom = *m_part->projectDom();
	if(DomUtil::readBoolEntry(dom,"/cppsupportpart/filetemplates/choosefiles",false))
		istr = FileTemplate::read(m_part, DomUtil::readEntry(dom,"/cppsupportpart/filetemplates/implementationURL",""), FileTemplate::Custom);
		else
		istr = FileTemplate::read(m_part, "cpp");
	}
	if (objc) {
		istr += QString("\n"
					"#include \"$HEADER$\"\n"
					"@implementation $CLASSNAME$\n"
					"@end\n");
	} else {
		istr += QString("\n"
					"#include \"$HEADER$\"\n"
					"\n"
					"\n"
					"$CLASSNAME$::$CLASSNAME$($ARGS$)\n"
					"$BASEINITIALIZER$\n"
					"{\n"
					"}\n"
					"\n"
					"\n"
					"$CLASSNAME$::~$CLASSNAME$()\n"
					"{\n"
					"}\n");
	}

	QString relPath;
	for (int i = implementation.findRev('/'); i != -1; i = implementation.findRev('/', --i))
		relPath += "../";
	QString args = childClass? "QWidget *parent, const char *name" : "";
	QString baseInitializer;
	if (!baseName.isEmpty())
		baseInitializer = childClass? "  : $BASECLASS$(parent, name)" : "  : $BASECLASS$()";

	istr.replace(QRegExp("\\$HEADER\\$"), relPath+header);
	istr.replace(QRegExp("\\$BASEINITIALIZER\\$"), baseInitializer);
	istr.replace(QRegExp("\\$CLASSNAME\\$"), className);
	istr.replace(QRegExp("\\$BASECLASS\\$"), baseName);
	istr.replace(QRegExp("\\$ARGS\\$"), args);
	istr.replace(QRegExp("\\$FILENAME\\$"), implementation);

	QFile ifile(implementationPath);
	if (!ifile.open(IO_WriteOnly)) {
		KMessageBox::error(this, i18n("Cannot write to implementation file"));
		return;
	}
	QTextStream istream(&ifile);
	istream << istr;
	ifile.close();

	QString hstr;
	if (filetemplate_box->isChecked()) {
	QDomDocument dom = *m_part->projectDom();
	if(DomUtil::readBoolEntry(dom,"/cppsupportpart/filetemplates/choosefiles",false))
		hstr = FileTemplate::read(m_part, DomUtil::readEntry(dom,"/cppsupportpart/filetemplates/interfaceURL",""), FileTemplate::Custom);
	else
		hstr = FileTemplate::read(m_part, "h");
	}

	if (objc) {
		hstr += QString("\n"
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
		hstr += QString("\n"
						"#ifndef _$HEADERGUARD$_\n"
						"#define _$HEADERGUARD$_\n"
						"\n"
						"$INCLUDEBASEHEADER$\n"
						"\n"
						"/**\n"
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
						"\n"
						"#endif\n"
						);
	}

	QString headerGuard = header.upper();
	headerGuard.replace(QRegExp("\\."),"_");
	QString includeBaseHeader;
	if (childClass)
		includeBaseHeader = "#include <qwidget.h>";
	else if (objc) {
		if (baseName != "NSObject")
			includeBaseHeader = "#include \"" + baseName + ".h\"";
	} else if (!baseName.isEmpty())
		includeBaseHeader = "#include <" + baseName.lower() + ".h>";

	QString author = DomUtil::readEntry(*m_part->projectDom(), "/general/author");

	QString inheritance;
	if (!baseName.isEmpty()) {
		inheritance += " : ";
		if (virtual_box->isChecked())
			inheritance += "virtual ";
		if (public_button->isChecked())
			inheritance += "public ";
		if (protected_button->isChecked())
			inheritance += "protected ";
		if (private_button->isChecked())
			inheritance += "private ";
		inheritance += baseName;
	}

	QString qobject;
	if (childClass)
		qobject = "  Q_OBJECT\n";

	hstr.replace(QRegExp("\\$HEADERGUARD\\$"), headerGuard);
	hstr.replace(QRegExp("\\$INCLUDEBASEHEADER\\$"), includeBaseHeader);
	hstr.replace(QRegExp("\\$AUTHOR\\$"), author);
	hstr.replace(QRegExp("\\$DOC\\$"), doc);
	hstr.replace(QRegExp("\\$CLASSNAME\\$"), className);
	hstr.replace(QRegExp("\\$BASECLASS\\$"), baseName);
	hstr.replace(QRegExp("\\$INHERITANCE\\$"), inheritance);
	hstr.replace(QRegExp("\\$QOBJECT\\$"), qobject);
	hstr.replace(QRegExp("\\$ARGS\\$"), args);
	hstr.replace(QRegExp("\\$FILENAME\\$"), header);

	QFile hfile(headerPath);
	if (!hfile.open(IO_WriteOnly)) {
		KMessageBox::error(this, i18n("Cannot write to header file"));
		return;
	}
	QTextStream hstream(&hfile);
	hstream << hstr;
	hfile.close();

	kdDebug(9007) << "Adding to project " << endl;
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
/*	project->addFile(project->activeDirectory() + "/" + header);
	project->addFile(project->activeDirectory() + "/" + implementation);*/
	kdDebug(9007) << "Added to project " << endl;

	QDialog::accept();
}

#include "cppnewclassdlg.moc"
