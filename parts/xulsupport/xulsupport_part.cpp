/**
 * xulsupport_part.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */
#include "xulsupport_part.h"

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevlanguagesupport.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>
#include <kdevappfrontend.h>
#include <domutil.h>
#include <codemodel.h>

#include <kparts/part.h>
#include <kapplication.h>
#include <kregexp.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kaction.h>
#include <kdialogbase.h>

#include <qwhatsthis.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qregexp.h>

typedef KGenericFactory<XulSupportPart> XulSupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevxulsupport, XulSupportFactory( "kdevxulsupport" ) );

XulSupportPart::XulSupportPart( QObject *parent, const char *name, const QStringList & )
  : KDevLanguageSupport( QString( "KDevPart" ), QString( "kdevpart" ), parent, name ? name : "XulSupportPart" )
{
	kdDebug()<<"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"<<endl;
	setInstance( XulSupportFactory::instance() );
	setXMLFile( "kdevxulsupport.rc" );
	initActions();
	initConnections();
	m_cc = new XulCodeCompletion();
}

XulSupportPart::~XulSupportPart()
{
	delete m_cc;
	m_cc = 0;
}

void
XulSupportPart::initActions()
{
	new KAction( i18n( "&View" ), "new", Key_F9, this, SLOT(slotView()),
							 actionCollection(), "build_view" );
}

void
XulSupportPart::initConnections()
{
	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
					 this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
	connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
	connect( partController(), SIGNAL(savedFile(const QString&)), this, SLOT(savedFile(const QString&)) );
	connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
					 this, SLOT(slotActivePartChanged(KParts::Part *)));
}

void
XulSupportPart::projectConfigWidget( KDialogBase* dlg )
{
	//TODO: Maybe some options would be nice?
	Q_UNUSED( dlg );
}

void
XulSupportPart::projectOpened()
{
  //kdDebug() << "projectOpened()" << endl;

	connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
					 this, SLOT(addedFilesToProject(const QStringList &)) );
	connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
					 this, SLOT(removedFilesFromProject(const QStringList &)) );

  // We want to parse only after all components have been
  // properly initialized
	QTimer::singleShot( 0, this, SLOT(parse()) );
}

void
XulSupportPart::projectClosed()
{
}

void
XulSupportPart::slotView()
{
	QString file;
	KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>( partController()->activePart() );
	if( ro_part )
		file = ro_part->url().path();

  QString cmd = QString::fromLatin1( "uxul" ) + QString::fromLatin1( " " ) + file;
  startApplication( cmd );
}

void
XulSupportPart::parse()
{
	kdDebug(9014) << "initialParse()" << endl;

	if ( project() )
	{
		kapp->setOverrideCursor( waitCursor );
		QStringList files = project()->allFiles();
		for ( QStringList::Iterator it = files.begin(); it != files.end() ;++it )
		{
			kdDebug(9014) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
			parse( project()->projectDirectory() + "/" + *it );
		}
		emit updatedSourceInfo();
		kapp->restoreOverrideCursor();
	} else {
		kdDebug(9014) << "No project" << endl;
	}
}

void
XulSupportPart::addedFilesToProject( const QStringList &fileList )
{
	kdDebug(9014) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		parse( project()->projectDirectory() + "/" + ( *it ) );
	}

	emit updatedSourceInfo();
}


void
XulSupportPart::removedFilesFromProject( const QStringList& fileList )
{
	kdDebug(9014) << "removedFilesFromProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		QString fileName = project()->projectDirectory() + "/" + ( *it );
		if( codeModel()->hasFile( fileName ) ){
			emit aboutToRemoveSourceInfo( fileName );
			codeModel()->removeFile( codeModel()->fileByName( fileName ) );
		}
	}
  //emit updatedSourceInfo();
}

void
XulSupportPart::savedFile(const QString &fileName)
{
	//kdDebug() << "savedFile()" << endl;

	if ( project()->allFiles().contains(
				 fileName.mid( project()->projectDirectory().length() + 1 ) ) )
	{
		parse( fileName );
		emit addedSourceInfo( fileName );
	}
}

void
XulSupportPart::startApplication( const QString &program )
{
	kdDebug() << "starting application" << program << endl;
	appFrontend()->startAppCommand( QString::QString(), program, true );
}


KDevLanguageSupport::Features
XulSupportPart::features()
{
	return Features( Variables | Classes );
}

void
XulSupportPart::parse( const QString& fileName )
{
	QFileInfo fi( fileName );
	m_vars.clear();

	if ( fi.extension() == "xul" )
	{
		if( codeModel()->hasFile( fileName ) ){
			emit aboutToRemoveSourceInfo( fileName );
			codeModel()->removeFile( codeModel()->fileByName( fileName ) );
		}

		FileDom m_file = codeModel()->create<FileModel>();
		m_file->setName( fileName );

		m_vars.clear();
		QFile f( QFile::encodeName( fileName ) );
		if ( !f.open( IO_ReadOnly ) )
			return;
		QString rawline;
		QString line;
		uint lineNo = 0;

		QDomDocument doc;

		if ( !doc.setContent( &f ) ) {
			f.close();
			return;
		}
		f.close();

		QDomElement docElem = doc.documentElement();

		QStringList completionTypes;

		QDomNode n = docElem.firstChild();
		while( !n.isNull() ) {
			if ( n.hasChildNodes() ) {
				QDomElement e = n.toElement(); // try to convert the node to an element.

				if( !e.isNull() ) {
					ClassDom cls = codeModel()->create<ClassModel>();
					cls->setStartPosition( lineNo, 0 );
					cls->setFileName( fileName );
					cls->setName( e.tagName() );
					if( !m_file->hasClass( cls->name() ) ) {
						kdDebug() << "Add global method " << cls->name() << endl;
						m_file->addClass( cls );
					}
					kdDebug() << e.tagName() << endl; // the node really is an element.
				}
			} else {
				//variable
			}
			++lineNo;
			n = n.nextSibling();
		}

		VariableList attrList = codeModel()->globalNamespace()->variableList();
		for (VariableList::Iterator it = attrList.begin(); it != attrList.end(); ++it)
			m_vars.append((*it)->name());

		m_cc->setVars(m_vars);

		codeModel()->addFile( m_file );
	}

}

void
XulSupportPart::slotActivePartChanged( KParts::Part *part )
{
	kdDebug() << "Changeing part..." << endl;
	m_cc->setActiveEditorPart( part );
}


XulCodeCompletion::XulCodeCompletion()
{
	m_argWidgetShow = false;
	m_completionBoxShow = false;
}

XulCodeCompletion::~XulCodeCompletion()
{
}

void
XulCodeCompletion::setActiveEditorPart( KParts::Part *part )
{
	if ( !part || !part->widget() )
		return;

	kdDebug() << "XulCodeCompletion::setActiveEditorPart"  << endl;


	m_editInterface = dynamic_cast<KTextEditor::EditInterface*>( part );

	if ( !m_editInterface )
	{
		kdDebug() << "editor doesn't support the EditDocumentIface" << endl;
		return;
	}

	m_cursorInterface = dynamic_cast<KTextEditor::ViewCursorInterface*>( part->widget() );
	if ( !m_cursorInterface )
	{
		kdDebug() << "editor does not support the ViewCursorInterface" << endl;
		return;
	}

	m_codeInterface = dynamic_cast<KTextEditor::CodeCompletionInterface*>( part->widget() );
	if ( !m_codeInterface ) { // no CodeCompletionDocument available
		kdDebug() << "editor doesn't support the CodeCompletionDocumentIface" << endl;
		return;
	}

	disconnect( part->widget(), 0, this, 0 ); // to make sure that it is't connected twice
	connect( part->widget(), SIGNAL(cursorPositionChanged()),
		this, SLOT(cursorPositionChanged()) );
	connect( part->widget(), SIGNAL(argHintHidden()), this, SLOT(argHintHidden()) );
	connect( part->widget(), SIGNAL(completionAborted()), this, SLOT(completionBoxAbort()) );
	connect( part->widget(), SIGNAL(completionDone()), this, SLOT(completionBoxHidden()) );
}

void
XulCodeCompletion::setVars( QStringList lst )
{
	m_vars = lst;
}

QValueList<KTextEditor::CompletionEntry>
XulCodeCompletion::getVars( const QString &startText )
{
	kdDebug() << "getVars" << endl;

	QValueList<KTextEditor::CompletionEntry> varList;

	QValueList<QString>::ConstIterator it;

	for ( it = m_vars.begin(); it != m_vars.end(); ++it ) {
		QString var = "$" + (*it);
		if ( var.startsWith( startText ) )
		{
			KTextEditor::CompletionEntry e;
			e.text = var;
			varList.append( e );
		}
	}

	return varList;
}

void
XulCodeCompletion::cursorPositionChanged()
{
	uint line, col;
	m_cursorInterface->cursorPositionReal( &line, &col );
	kdDebug() << "XulCodeCompletion::cursorPositionChanged:" << line << ":" << col  << endl;

	QString lineStr = m_editInterface->textLine( line );
	if( lineStr.isNull() || lineStr.isEmpty() ){
		kdDebug() << "No Text..." << endl;
		return; // nothing to do
	}

	QString restLine = lineStr.mid( col );
	QString prevText = lineStr.mid( 0,col );

	if( restLine.left(1) != " " && restLine.left(1) != "\t" && !restLine.isNull() )
	{
		kdDebug() << "no codecompletion because no empty character after cursor:" << restLine << ":" << endl;
		return;
	}

	KRegExp prevReg("\\<[\\d\\w]\\b$" );

	if ( prevReg.match( prevText.local8Bit() ) )
	{
		kdDebug()<<"### MATCHED!"<<endl;
		// We are in completion mode

		QString startMatch = prevReg.group(0);

		kdDebug() << "Matching: " << startMatch << endl;

		m_completionBoxShow = true;
		m_codeInterface->showCompletionBox( getVars( startMatch ), 2 );
	}
	else
	{
		kdDebug() << "no vars in: " << prevText << endl;
		return;
	}
}

void
XulCodeCompletion::completionBoxHidden()
{
	kdDebug() << "Complete..." << endl;
	m_completionBoxShow = false;
}

void
XulCodeCompletion::completionBoxAbort()
{
	kdDebug() << "aborted..." << endl;
	m_completionBoxShow = false;
}

KMimeType::List
XulSupportPart::mimeTypes( )
{
	KMimeType::List list;

	KMimeType::Ptr mime = KMimeType::mimeType( "application/vnd.mozilla.xul+xml" );
	if ( mime )
		list << mime;

	return list;
}

#include "xulsupport_part.moc"
