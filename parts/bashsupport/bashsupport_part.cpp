/*
* $Id$
*  Copyright (C) 2003 Ian Reinhart Geiser <geiseri@kde.org>
*/
#include <qwhatsthis.h>

#include <qfileinfo.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <kapplication.h>
#include <kregexp.h>
#include <qregexp.h>

#include "classstore.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kaction.h>
#include <kparts/part.h>
#include <kdialogbase.h>


#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevlanguagesupport.h"
#include "kdevpartcontroller.h"
#include "kdevproject.h"
#include "kdevappfrontend.h"

#include "bashsupport_part.h"
#include "domutil.h"

typedef KGenericFactory<BashSupportPart> BashSupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevbashsupport, BashSupportFactory( "kdevbashsupport" ) );

BashSupportPart::BashSupportPart(QObject *parent, const char *name, const QStringList& )
: KDevLanguageSupport ("KDevPart", "kdevpart", parent, name ? name : "BashSupportPart" )
{
	setInstance(BashSupportFactory::instance());
	setXMLFile("kdevbashsupport.rc");

	KAction *action;
	action = new KAction( i18n("&Run"), "exec",Key_F9,this, SLOT(slotRun()),actionCollection(), "build_execute" );

	kdDebug() << "Creating BashSupportPart" << endl;

	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
		this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
	connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
	connect( partController(), SIGNAL(savedFile(const QString&)), this, SLOT(savedFile(const QString&)) );
 	connect(partController(), SIGNAL(activePartChanged(KParts::Part*)),
		this, SLOT(slotActivePartChanged(KParts::Part *)));

	m_cc = new BashCodeCompletion();
}


BashSupportPart::~BashSupportPart()
{

}


void BashSupportPart::projectConfigWidget(KDialogBase *dlg)
{
//	QVBox *vbox = dlg->addVBoxPage(i18n("Bash"));
//	RubyConfigWidget *w = new RubyConfigWidget(*projectDom(), (QWidget *)vbox, "Bash config widget");
//	connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void BashSupportPart::projectOpened()
{
	kdDebug(9014) << "projectOpened()" << endl;

	connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
		this, SLOT(addedFilesToProject(const QStringList &)) );
	connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
		this, SLOT(removedFilesFromProject(const QStringList &)) );

	// We want to parse only after all components have been
	// properly initialized
	QTimer::singleShot(0, this, SLOT(parse()));
}


void BashSupportPart::projectClosed()
{
}

void BashSupportPart::slotRun ()
{
	QString file;
	KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
	if(ro_part)
		file = ro_part->url().path();

	QString cmd = interpreter() + " " + file;
	startApplication(cmd);
}

QString BashSupportPart::interpreter()
{
	QString prog = DomUtil::readEntry(*projectDom(), "/kdevrbashsupport/run/interpreter");
	if (prog.isEmpty())
		prog = "bash";
	return prog;
}

void BashSupportPart::parse()
{
	kdDebug(9014) << "initialParse()" << endl;

	if (project())
	{
		kapp->setOverrideCursor(waitCursor);
		QStringList files = project()->allFiles();
		for (QStringList::Iterator it = files.begin(); it != files.end() ;++it)
		{
			kdDebug(9014) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
			parse(project()->projectDirectory() + "/" + *it);
		}
		emit updatedSourceInfo();
		kapp->restoreOverrideCursor();
	} else {
		kdDebug(9014) << "No project" << endl;
	}
}

void BashSupportPart::addedFilesToProject(const QStringList &fileList)
{
kdDebug(9014) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		parse(project()->projectDirectory() + "/" + ( *it ) );
	}

emit updatedSourceInfo();
}


void BashSupportPart::removedFilesFromProject(const QStringList &fileList)
{
	kdDebug(9014) << "removedFilesFromProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		classStore()->removeWithReferences(project()->projectDirectory() + "/" + ( *it ) );
	}

	emit updatedSourceInfo();
}

void BashSupportPart::savedFile(const QString &fileName)
{
	kdDebug(9014) << "savedFile()" << endl;

	if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 )))
	{
		parse(fileName);
		emit updatedSourceInfo();
	}
}

void BashSupportPart::startApplication(const QString &program)
{
	kdDebug() << "starting application" << program << endl;
	appFrontend()->startAppCommand(QString::QString(), program, TRUE);
}


KDevLanguageSupport::Features BashSupportPart::features()
{
	return Features(Variables | Functions);
}


QStringList BashSupportPart::fileFilters()
{
	QStringList l;
	l << "*.sh";
	return l;
}

void BashSupportPart::parse(const QString &fileName)
{
	QFileInfo fi(fileName);
	m_vars.clear();
	if (fi.extension() == "sh")
	{
		classStore()->removeWithReferences(fileName);
		m_vars.clear();
		QFile f(QFile::encodeName(fileName));
		if (!f.open(IO_ReadOnly))
			return;
		QString rawline;
		QString line;
		uint lineNo = 0;
		KRegExp methodre("\\b([\\d\\w]+[\\s]*)\\([\\s]*\\)");
		KRegExp varre("\\b([\\d\\w]+)([=])");
		KRegExp forvarre("\\b(for\\b[\\s]+)(\\b[\\d\\w]+\\b)([\\s]+in\\b[\\s]+\\$)");

		QTextStream stream(&f);
		while (!stream.atEnd())
		{
			 rawline = stream.readLine();
       			 line = rawline.stripWhiteSpace().local8Bit();
			 kdDebug() << "Trying line: " << line << endl;
			 if (methodre.match(line))
			 {
				ParsedMethod *method = new ParsedMethod;
				method->setName(methodre.group(1));
				method->setDefinedInFile(fileName);
				method->setDefinedOnLine(lineNo);

				ParsedMethod *old = classStore()->globalScope()->getMethod(method);

				if( old )
				{
					delete( method );
					method = old;
				}
				else
				{
					kdDebug() << "Add global method " << method->name() << endl;
					classStore()->globalScope()->addMethod(method);
				}
			}
			else if(varre.match(line))
			{
				addAttribute(varre.group(1), fileName ,lineNo);
			}
			else if(forvarre.match(line))
			{
				addAttribute(forvarre.group(2), fileName ,lineNo);
			}
			++lineNo;
		}
		f.close();

		QValueList<ParsedAttribute*> attrList = classStore()->globalScope()->getSortedAttributeList();
		QValueList<ParsedAttribute*>::ConstIterator it;
		for (it = attrList.begin(); it != attrList.end(); ++it)
			m_vars.append((*it)->name());
			m_cc->setVars(m_vars);

	}
}

void BashSupportPart::slotActivePartChanged(KParts::Part *part)
{
	kdDebug() << "Changeing part..." << endl;
	m_cc->setActiveEditorPart(part);
}

void BashSupportPart::addAttribute(const QString &name, const QString &fileName, uint lineNo)
{
	ParsedAttribute *var = new ParsedAttribute;
	var->setName(name);
	var->setDefinedInFile(fileName);
	var->setDefinedOnLine(lineNo);
	var->setDefinedInFile(fileName);
	var->setDefinedOnLine(lineNo);
	var->setType(i18n("Variable"));
	ParsedAttribute *old = classStore()->globalScope()->getAttributeByName(var->name());

	if( old )
	{
		delete( var );
		var = old;
	}
	else
	{
		kdDebug() << "Add global attribute " << var->name() << endl;
		classStore()->globalScope()->addAttribute(var);
	}
}

BashCodeCompletion::BashCodeCompletion()
{
	m_argWidgetShow = false;
	m_completionBoxShow=false;
}

BashCodeCompletion::~BashCodeCompletion()
{

}

void BashCodeCompletion::setActiveEditorPart(KParts::Part *part)
{
	if (!part || !part->widget())
		return;

	kdDebug() << "BashCodeCompletion::setActiveEditorPart"  << endl;

// We need to think about this
//	if(!(m_config->getCodeCompletion() || m_config->getCodeHinting())){
//		return; // no help
//	}

	m_editInterface = dynamic_cast<KTextEditor::EditInterface*>(part);
	if (!m_editInterface)
	{
		kdDebug() << "editor doesn't support the EditDocumentIface" << endl;
		return;
	}

	m_cursorInterface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
	if (!m_cursorInterface)
	{
		kdDebug() << "editor does not support the ViewCursorInterface" << endl;
		return;
	}

	m_codeInterface = dynamic_cast<KTextEditor::CodeCompletionInterface*>(part->widget());
	if (!m_codeInterface) { // no CodeCompletionDocument available
		kdDebug() << "editor doesn't support the CodeCompletionDocumentIface" << endl;
		return;
	}

	disconnect(part->widget(), 0, this, 0 ); // to make sure that it is't connected twice
	connect(part->widget(), SIGNAL(cursorPositionChanged()),
		this, SLOT(cursorPositionChanged()));
	connect(part->widget(), SIGNAL(argHintHidden()), this, SLOT(argHintHidden()));
	connect(part->widget(), SIGNAL(completionAborted()), this, SLOT(completionBoxAbort()));
	connect(part->widget(), SIGNAL(completionDone()), this, SLOT(completionBoxHidden()));

}

void BashCodeCompletion::setVars(QStringList vars)
{
	kdDebug() << "setVars" << endl;
	m_vars.clear();
	QValueList<QString>::ConstIterator it;
	for (it = vars.begin(); it != vars.end(); ++it) {
		KTextEditor::CompletionEntry e;
		e.text = (*it);
		e.postfix ="";
		e.prefix ="$";
		m_vars.append(e);
	}
}

void BashCodeCompletion::cursorPositionChanged()
{
	uint line, col;
	m_cursorInterface->cursorPositionReal(&line, &col);
	kdDebug() << "BashCodeCompletion::cursorPositionChanged:" << line << ":" << col  << endl;

	QString lineStr = m_editInterface->textLine(line);
	if(lineStr.isNull() || lineStr.isEmpty()){
		kdDebug() << "No Text..." << endl;
		return; // nothing to do
	}
//	if(m_config->getCodeCompletion())
//	{
		QString restLine = lineStr.mid(col);
		QString prevText = lineStr.mid(0,col);

		if(restLine.left(1) != " " && restLine.left(1) != "\t" && !restLine.isNull())
		{
			kdDebug() << "no codecompletion because no empty character after cursor:" << restLine << ":" << endl;
			return;
		}

		if ( prevText.findRev(QRegExp("[$][\\d\\w]*\\b$")) == -1)
		{
			kdDebug() << "no vars " << prevText << endl;
			return;
			// We have a variable
		}
		m_completionBoxShow=true;
		m_codeInterface->showCompletionBox(m_vars);

//	}

}

void BashCodeCompletion::completionBoxHidden()
{
	kdDebug() << "Complete..." << endl;
	m_completionBoxShow=false;
	uint line, col, start;
	m_cursorInterface->cursorPositionReal(&line, &col);
	QString lineStr = m_editInterface->textLine(line);
	start = lineStr.findRev(QRegExp("[$][\\d\\w]*\\b$"));
	m_editInterface->removeText ( start, col, line, col );
}

void BashCodeCompletion::completionBoxAbort()
{
	kdDebug() << "aborted..." << endl;
	m_completionBoxShow=false;
}
#include "bashsupport_part.moc"
