/*
*  Copyright (C) 2003 Ian Reinhart Geiser <geiseri@kde.org>
*/
#include "bashsupport_part.h"

#include <q3whatsthis.h>

#include <qfileinfo.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <kapplication.h>
#include <qregexp.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kaction.h>
#include <kparts/part.h>
#include <kdialogbase.h>


#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevlanguagesupport.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>
#include <kdevappfrontend.h>
#include <kdevplugininfo.h>
#include <domutil.h>
#include <codemodel.h>

typedef KDevGenericFactory<BashSupportPart> BashSupportFactory;
static const KDevPluginInfo data("kdevbashsupport");
K_EXPORT_COMPONENT_FACTORY( libkdevbashsupport, BashSupportFactory( data ) )

BashSupportPart::BashSupportPart(QObject *parent, const char *name, const QStringList& )
: KDevLanguageSupport (&data, parent, name ? name : "BashSupportPart" )
{
	setInstance(BashSupportFactory::instance());
	setXMLFile("kdevbashsupport.rc");

	KAction *action;
	action = new KAction( i18n("&Run"), "exec",Qt::Key_F9,this, SLOT(slotRun()),actionCollection(), "build_execute" );
    action->setToolTip(i18n("Run"));
    action->setWhatsThis(i18n("<b>Run</b><p>Starts an application."));

	kdDebug() << "Creating BashSupportPart" << endl;

	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
		this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
	connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
	connect( partController(), SIGNAL(savedFile(const KURL&)), this, SLOT(savedFile(const KURL&)) );
 	connect(partController(), SIGNAL(activePartChanged(KParts::Part*)),
		this, SLOT(slotActivePartChanged(KParts::Part *)));

	m_cc = new BashCodeCompletion();
}


BashSupportPart::~BashSupportPart()
{
    delete( m_cc );
    m_cc = 0;
}


void BashSupportPart::projectConfigWidget(KDialogBase *dlg)
{
    Q_UNUSED( dlg );
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
		kapp->setOverrideCursor(Qt::WaitCursor);
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
		QString fileName = project()->projectDirectory() + "/" + ( *it );
		if( codeModel()->hasFile(fileName) ){
		    emit aboutToRemoveSourceInfo( fileName );
		    codeModel()->removeFile( codeModel()->fileByName(fileName) );
		}
	}

	//emit updatedSourceInfo();
}

void BashSupportPart::savedFile(const KURL &fileName)
{
	kdDebug(9014) << "savedFile()" << endl;

	if (project()->allFiles().contains(fileName.path().mid ( project()->projectDirectory().length() + 1 )))
	{
		parse(fileName.path());
		emit addedSourceInfo( fileName.path() );
	}
}

void BashSupportPart::startApplication(const QString &program)
{
	kdDebug() << "starting application" << program << endl;
	if (KDevAppFrontend *appFrontend = extension<KDevAppFrontend>("KDevelop/AppFrontend"))
		appFrontend->startAppCommand(QString::QString(), program, TRUE);
}


KDevLanguageSupport::Features BashSupportPart::features()
{
	return Features(Variables | Functions);
}

void BashSupportPart::parse(const QString &fileName)
{
	QFileInfo fi(fileName);
	m_vars.clear();
	if (fi.extension() == "sh")
	{
		if( codeModel()->hasFile(fileName) ){
		    emit aboutToRemoveSourceInfo( fileName );
		    codeModel()->removeFile( codeModel()->fileByName(fileName) );
		}

		FileDom m_file = codeModel()->create<FileModel>();
		m_file->setName( fileName );

		m_vars.clear();
		QFile f(QFile::encodeName(fileName));
		if (!f.open(QIODevice::ReadOnly))
			return;
		QString rawline;
		QString line;
		uint lineNo = 0;
		//KRegExp methodre("\\b([\\d\\w]+[\\s]*)\\([\\s]*\\)");
		QRegExp methodre("^\\s*(\\w+)\\s*\\(\\s*\\)");
		QRegExp varre( "^\\s*(\\w+)[=]" );
		QRegExp expvarre( "^export\\s*(\\w+)[=]" );
		QRegExp forvarre("\\bfor[\\s]+([\\d\\w]+)[\\s]+in[\\s]+");

		QTextStream stream(&f);
		while (!stream.atEnd())
		{
			 rawline = stream.readLine();
       			 line = rawline.stripWhiteSpace().local8Bit();
			 kdDebug() << "Trying line: " << line << endl;
			 if (methodre.search(line) != -1)
			 {
				FunctionDom method = codeModel()->create<FunctionModel>();
				method->setName(methodre.cap(1));
				method->setFileName(fileName);
				method->setStartPosition(lineNo, 0);

				if( !m_file->hasFunction(method->name()) ){
					kdDebug() << "Add global method " << method->name() << endl;
					m_file->addFunction( method );
				}
			}
			else if(varre.search(line) != -1)
			{
				addAttribute(varre.cap(1), m_file, lineNo);
			}
			else if(expvarre.search(line) != -1)
			{
				addAttribute(expvarre.cap(1), m_file, lineNo);
			}
			else if(forvarre.search(line) != -1)
			{
				addAttribute(forvarre.cap(1), m_file, lineNo);
			}
			++lineNo;
		}
		f.close();

		kdDebug() << "Trying to add list..." << endl;
		codeModel()->addFile( m_file );
		VariableList attrList = codeModel()->globalNamespace()->variableList();
		for (VariableList::Iterator it = attrList.begin(); it != attrList.end(); ++it)
		{
			kdDebug() << "Adding " << (*it)->name() << endl;
			m_vars.append((*it)->name());
		}
		m_cc->setVars(m_vars);

		codeModel()->addFile( m_file );
	}

}

void BashSupportPart::slotActivePartChanged(KParts::Part *part)
{
	kdDebug() << "Changeing part..." << endl;
	m_cc->setActiveEditorPart(part);
}

void BashSupportPart::addAttribute(const QString &name, FileDom file, uint lineNo)
{
	VariableDom var = codeModel()->create<VariableModel>();
	var->setName(name);
	var->setFileName(file->name());
	var->setStartPosition( lineNo, 0 );
	var->setType(i18n("Variable"));

	if( !file->hasVariable(var->name()) ){
		kdDebug() << "Add global attribute " << var->name() << endl;
		file->addVariable(var);
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

void BashCodeCompletion::setVars(QStringList lst)
{
	m_vars = lst;
}

Q3ValueList<KTextEditor::CompletionEntry> BashCodeCompletion::getVars(const QString &startText)
{
	kdDebug() << "getVars for " << startText << endl;
	Q3ValueList<KTextEditor::CompletionEntry> varList;
	Q3ValueList<QString>::ConstIterator it;
	for (it = m_vars.begin(); it != m_vars.end(); ++it) {
		QString var = "$" + (*it);
		kdDebug() << "Compair " << var << endl;
		if( var.startsWith( startText ))
		{
		  KTextEditor::CompletionEntry e;
		  e.text = var;
		  //e.postfix ="";
		  //e.prefix ="";
		  kdDebug() << "getVar: " << var << endl;
		  varList.append(e);
		}
	}

	return varList;
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

		QRegExp prevReg("[$][\\d\\w]*\\b$");

		int pos = prevReg.search( prevText );
		if (pos > -1 )
		{
			// We are in completion mode
		      QString startMatch = prevReg.cap(0);
		      kdDebug() << "Matching: " << startMatch << endl;
		      m_completionBoxShow=true;
		      m_codeInterface->showCompletionBox(getVars(startMatch),2);
		}
		else
		{
			kdDebug() << "no vars in: " << prevText << endl;
			return;
		}

//	}

}

void BashCodeCompletion::completionBoxHidden()
{
	kdDebug() << "Complete..." << endl;
	m_completionBoxShow=false;
/*	uint line, col, start;
	m_cursorInterface->cursorPositionReal(&line, &col);
	QString lineStr = m_editInterface->textLine(line);

	start = lineStr.findRev(QRegExp("[$][\\d\\w]*\\b$"));
	m_editInterface->removeText ( start, col, line, col );
*/
}

void BashCodeCompletion::completionBoxAbort()
{
	kdDebug() << "aborted..." << endl;
	m_completionBoxShow=false;
}

KMimeType::List BashSupportPart::mimeTypes( )
{
    KMimeType::List list;

    KMimeType::Ptr mime = KMimeType::mimeType( "application/x-shellscript" );
    if( mime )
	list << mime;

    return list;
}
#include "bashsupport_part.moc"
