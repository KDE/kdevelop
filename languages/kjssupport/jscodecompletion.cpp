//
// C++ Implementation: jscodecompletion
//
// Description:
//
//
// Author: ian reinhart geiser <geiseri@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "jscodecompletion.h"
#include <qwhatsthis.h>

#include <qfileinfo.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <kapplication.h>
#include <qregexp.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kaction.h>
#include <kparts/part.h>
#include <kdialogbase.h>


#include <kdevelop/kdevcore.h>
#include <kdevelop/kdevmainwindow.h>
#include <kdevelop/kdevlanguagesupport.h>
#include <kdevelop/kdevpartcontroller.h>
#include <kdevelop/kdevproject.h>
#include <kdevelop/kdevappfrontend.h>
#include <kdevelop/domutil.h>
#include <kdevelop/codemodel.h>

JSCodeCompletion::JSCodeCompletion(QObject *parent, const char *name)
		: QObject(parent, name)
{
	m_argWidgetShow = false;
	m_completionBoxShow=false;
}


JSCodeCompletion::~JSCodeCompletion()
{}

void JSCodeCompletion::setActiveEditorPart( KParts::Part * part )
{
	if (!part || !part->widget())
		return;

	kdDebug() << "JSCodeCompletion::setActiveEditorPart"  << endl;

	// We need to think about this
	//      if(!(m_config->getCodeCompletion() || m_config->getCodeHinting())){
	//              return; // no help
	//      }

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
	if (!m_codeInterface)
	{ // no CodeCompletionDocument available
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

QValueList< KTextEditor::CompletionEntry > JSCodeCompletion::getVars( const QString & startText )
{
	kdDebug() << "getVars for " << startText << endl;
	QValueList<KTextEditor::CompletionEntry> varList;
	/*
	QValueList<QString>::ConstIterator it;
	for (it = m_vars.begin(); it != m_vars.end(); ++it)
	{
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
	*/
	return varList;
}

void JSCodeCompletion::cursorPositionChanged( )
{
	uint line, col;
	m_cursorInterface->cursorPositionReal(&line, &col);
	kdDebug() << "JSCodeCompletion::cursorPositionChanged:" << line << ":" << col  << endl;

	QString lineStr = m_editInterface->textLine(line);
	if(lineStr.isNull() || lineStr.isEmpty())
	{
		kdDebug() << "No Text..." << endl;
		return; // nothing to do
	}
	//      if(m_config->getCodeCompletion())
	//      {
	QString restLine = lineStr.mid(col);
	QString prevText = lineStr.mid(0,col);

	if(restLine.left(1) != " " && restLine.left(1) != "\t" && !restLine.isNull())
	{
		kdDebug() << "no codecompletion because no empty character after cursor:" << restLine << ":" << endl;
		return;
	}

	QRegExp prevReg("([\\d\\w]*)[.]$");

	if (prevReg.search( prevText ) != -1 )
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

	//      }


}

void JSCodeCompletion::completionBoxHidden( )
{
	kdDebug() << "Complete..." << endl;
	m_completionBoxShow=false;
}

void JSCodeCompletion::completionBoxAbort( )
{
	kdDebug() << "aborted..." << endl;
	m_completionBoxShow=false;

}


#include "jscodecompletion.moc"
