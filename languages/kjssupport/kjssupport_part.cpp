/* 
   Copyright (C) 2003 ian reinhart geiser <geiseri@kde.org> 

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qwhatsthis.h>
#include <qtimer.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qpopupmenu.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdevcore.h>
#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>
#include <domutil.h>
#include <codemodel.h>
#include <kparts/part.h>


#include <kdevproject.h>
#include <kaction.h>
#include <kdebug.h>
#include <kapplication.h>

#include <kjsembed/kjsembedpart.h>
#include <kjsembed/jsconsolewidget.h>

#include <kdevplugininfo.h>

#include "kjssupport_part.h"
#include "kjsproblems.h"
#include "jscodecompletion.h"
#include "subclassingdlg.h"

typedef KDevGenericFactory<kjsSupportPart> kjsSupportFactory;
static const KDevPluginInfo data("kdevkjssupport");
K_EXPORT_COMPONENT_FACTORY( libkdevkjssupport, kjsSupportFactory( data ) );


class typeProperty
{
	public:
		QString type;
		QString name;
		int depth;
};

kjsSupportPart::kjsSupportPart(QObject *parent, const char *name, const QStringList& )
: KDevLanguageSupport(&data, parent, name ? name : "kjsSupportPart" )
{
	setInstance(kjsSupportFactory::instance());
	setXMLFile("kdevkjssupport.rc");


	m_build = new KAction( i18n("&Run"), "exec",Key_F9,this, SLOT(slotRun()),actionCollection(), "build_execute" );
	m_build->setStatusText( i18n("Test the active script.") );

	kdDebug() << "Creating kjssupport Part" << endl;

	connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
		this, SLOT(projectConfigWidget(KDialogBase*)) );
	connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
	connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
	connect( partController(), SIGNAL(savedFile(const QString&)), this, SLOT(savedFile(const QString&)) );
	connect(partController(), SIGNAL(activePartChanged(KParts::Part*)),
		this, SLOT(slotActivePartChanged(KParts::Part *)));
	connect(core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
		this, SLOT(contextMenu(QPopupMenu *, const Context *)));


     // Building kjs interpreter.
	m_js = new KJSEmbed::KJSEmbedPart();
	mainWindow()->embedOutputView( m_js->view() , i18n("KJS Console"),i18n("KJS Embed Console") );

    // get the problem reporter
        m_problemReporter = new KJSProblems( this, 0, "problems" );
	mainWindow( )->embedOutputView( m_problemReporter, i18n("Problems"), i18n("Problem reporter"));
	m_cc = new JSCodeCompletion();
    }


kjsSupportPart::~kjsSupportPart()
{
	delete m_problemReporter;
	delete m_cc;
	delete m_build;
	delete m_js;
}

KDevLanguageSupport::Features kjsSupportPart::features()
{
	return Features(Classes | Variables | Functions);
}

KMimeType::List kjsSupportPart::mimeTypes()
{
	KMimeType::List list;
	
	KMimeType::Ptr mime = KMimeType::mimeType( "application/x-javascript" );
	if( mime )
		list << mime;

	return list;
}
void kjsSupportPart::slotRun()
{
	// Execute the application here.

	KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( partController()->activePart() );
	if ( ro_part ) 
	{
		m_js->runFile( ro_part->url().path() );
	}
	
}

void kjsSupportPart::projectConfigWidget(KDialogBase *dlg)
{
	Q_UNUSED( dlg );
	// Create your config dialog here.
}
void kjsSupportPart::projectOpened()
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
void kjsSupportPart::projectClosed()
{

}
void kjsSupportPart::savedFile(const QString &fileName)
{


	if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 )))
	{
		kdDebug(9014) << "parse file " << fileName << endl;
		parse( fileName );
		emit addedSourceInfo( fileName );
	}
}
void kjsSupportPart::addedFilesToProject(const QStringList &fileList)
{
	kdDebug(9014) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		kdDebug(9014) << "maybe parse " << project()->projectDirectory() + "/" + ( *it ) << endl;
		parse( project()->projectDirectory() + "/" + (*it) );
	}

	emit updatedSourceInfo();
}
void kjsSupportPart::removedFilesFromProject(const QStringList &fileList)
{


	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		QString fileName = project()->projectDirectory() + "/" + ( *it );
		if( codeModel()->hasFile(fileName) )
		{
			kdDebug(9014) << "removed " << fileName << endl;
			emit aboutToRemoveSourceInfo( fileName );
			codeModel()->removeFile( codeModel()->fileByName(fileName) );
		}
	}

}
void kjsSupportPart::parse()
{
	kdDebug(9014) << "initialParse()" << endl;

	if (project())
	{
		kapp->setOverrideCursor(waitCursor);
		QStringList files = project()->allFiles();
		m_problemReporter->clear();

		for (QStringList::Iterator it = files.begin(); it != files.end() ;++it)
		{
			kdDebug(9014) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
			
			parse( project()->projectDirectory() + "/" + (*it) );
			
		}
		emit updatedSourceInfo();
		kapp->restoreOverrideCursor();
	} else {
		kdDebug(9014) << "No project" << endl;
	}
}
void kjsSupportPart::slotActivePartChanged(KParts::Part *part)
{
	kdDebug() << "Changeing active part..." << endl;
	m_cc->setActiveEditorPart(part);
}

/*!
    \fn kjsSupportPart::parse(const QString &fileName)
 */
void kjsSupportPart::parse(const QString &fileName)
{
	QFileInfo fi(fileName);
	if (fi.extension() == "js")
	{
		if( codeModel()->hasFile(fileName) )
		{
			emit aboutToRemoveSourceInfo( fileName );
			codeModel()->removeFile( codeModel()->fileByName(fileName) );
		}
	
		FileDom m_file = codeModel()->create<FileModel>();
		m_file->setName( fileName );
		m_file->setFileName( fileName );
		
                QFile f(QFile::encodeName(fileName));
                if (!f.open(IO_ReadOnly))
                        return;
                QString rawline;
                QString line;
                uint lineNo = 0;
		QTextStream stream(&f);
		int depth = 0;
		bool inFunction = false;
		QString lastFunction = "";
		int lastLineNo = 0;
		ClassDom currentClass;
		
		QRegExp varRx("var[\\s]([_a-zA-Z\\d]+)");
		QRegExp classVarRx("this\\.([_a-zA-Z\\d]+)");
		QRegExp classMethRx("this\\.([_a-zA-Z\\d]+)[\\s]*=[\\s]*function(\\([^){}\\n\\r]*\\))");
		QRegExp methRx("function[\\s]+([_a-zA-Z\\d]+[\\s]*\\([^){}\\n\\r]*\\))");
		QRegExp allocRx("([_\\d\\w]+)[\\s]*=[\\s]*new[\\s]*([_\\d\\w]+)");
		QRegExp assnRx("var[\\s]+([_\\d\\w]+)[\\s]+[=][\\s]+([_\\d\\w]+)[;]");
		
                while (!stream.atEnd())
                {
                         rawline = stream.readLine();
                         line = rawline.stripWhiteSpace().local8Bit();
                         kdDebug() << "Trying line: " << line << endl;
			 
			if (methRx.search(line) != -1 && depth == 0)
			{
				if (lastFunction != "" )
					addMethod(lastFunction, m_file, lastLineNo);
				lastFunction = methRx.cap(1);
				lastLineNo = lineNo;
                        }
                        else if(varRx.search(line) != -1 && depth == 0)
                        {
                                addAttribute(varRx.cap(1), m_file, lineNo);
                        }
                        else if(classMethRx.search(line) != -1 && depth > 0)
                        {
				if ( lastFunction != "" )
				{
					currentClass = addClass(lastFunction, m_file, lastLineNo );
					lastFunction = "";
				}
                               addMethod(classMethRx.cap(1)+classMethRx.cap(2), currentClass, lineNo);
                        }
                        else if(classVarRx.search(line) != -1 && depth > 0)
                        {
				if ( lastFunction != "" )
				{
					currentClass = addClass(lastFunction, m_file, lastLineNo );
					lastFunction = "";
				}
                                addAttribute(classVarRx.cap(1), currentClass, lineNo);
                        }
			
			if( allocRx.search(line) != -1 )
			{
				QString varName = allocRx.cap(1);
				QString varType = allocRx.cap(2);
				
				typeProperty *type = new typeProperty();
				type->depth = depth;
				type->name = varName;
				type->type = varType;
				
				m_typeMap.insert(varName, type);
				kdDebug() << "Adding " << varName << " of type " << varType << " at scope " << depth << endl;
				
			}
			
						
			kdDebug() << "Syntax check..." << endl;
			KJS::UString jsLine( line.latin1() );
			int lineNumber = 0;
			KJS::UString errorMessage;
	
			if ( !m_js->interpreter()->checkSyntax( jsLine, &lineNumber, &errorMessage ) )
			{
				kdDebug() << errorMessage.qstring() << " on line " << lineNo << endl;
				m_problemReporter->addLine(m_file->fileName(), lineNo, errorMessage.qstring());
			}

			if( line.contains("{") )
				++depth;
				
			if( line.contains("}") )
				--depth;

                       ++lineNo;
                }
		
		if (lastFunction != "" )
			addMethod(lastFunction, m_file, lastLineNo);
					
                f.close();

		kdDebug() << "Trying to add list..." << endl;
                
                codeModel()->addFile( m_file );

		
        }
}

ClassDom kjsSupportPart::addClass(const QString &name, FileDom file, uint lineNo)
{
 	ClassDom clazz = codeModel()->create<ClassModel>();
	clazz->setName(name);
	clazz->setFileName(file->fileName());
	clazz->setStartPosition(lineNo, 0);

	if( !file->hasClass(clazz->name()) ){
		kdDebug() << "Add global class " << clazz->name() << endl;
		file->addClass( clazz );
	}
	return clazz;
}

void kjsSupportPart::addMethod(const QString &name, ClassDom clazz, uint lineNo)
{
 	FunctionDom method = codeModel()->create<FunctionModel>();
	method->setName(name);
	method->setFileName(clazz->fileName());
	method->setStartPosition(lineNo, 0);

	if( !clazz->hasFunction(method->name()) ){
		kdDebug() << "Add class method " << method->name() << endl;
		clazz->addFunction( method );
	}
}

void kjsSupportPart::addAttribute(const QString &name, ClassDom clazz, uint lineNo)
{
	VariableDom var = codeModel()->create<VariableModel>();
	var->setName(name);
	var->setFileName(clazz->fileName());
	var->setStartPosition( lineNo, 0 );
	var->setType(i18n("Variable"));

	if( !clazz->hasVariable(var->name()) ){
		kdDebug() << "Add class attribute " << var->name() << endl;
		clazz->addVariable(var);
	}
}

void kjsSupportPart::addMethod(const QString &name, FileDom file, uint lineNo)
{
 	FunctionDom method = codeModel()->create<FunctionModel>();
	method->setName(name);
	method->setFileName(file->fileName());
	method->setStartPosition(lineNo, 0);

	if( !file->hasFunction(method->name()) ){
		kdDebug() << "Add global method " << method->name() << endl;
		file->addFunction( method );
	}
}

void kjsSupportPart::addAttribute(const QString &name, FileDom file, uint lineNo)
{
	VariableDom var = codeModel()->create<VariableModel>();
	var->setName(name);
	var->setFileName(file->fileName());
	var->setStartPosition( lineNo, 0 );
	var->setType(i18n("Variable"));

	if( !file->hasVariable(var->name()) ){
		kdDebug() << "Add global attribute " << var->name() << endl;
		file->addVariable(var);
	}
}

void kjsSupportPart::contextMenu(QPopupMenu * popupMenu, const Context *context)
{
	kdDebug() << "1" << endl;
	if (!context->hasType( Context::FileContext ))
		return;

	kdDebug() << "2" << endl;
	const FileContext *fcontext = static_cast<const FileContext*>(context);
	m_selectedUI = fcontext->fileName();
	if (m_selectedUI.right(3).lower() == ".ui")
		int id = popupMenu->insertItem(i18n("Implement Slots"),
			this, SLOT(implementSlots()));
	else
		m_selectedUI = QString::null;
}

void kjsSupportPart::implementSlots()
{
	if (m_selectedUI.isEmpty())
		return;
	
	QStringList newFiles;
	SubclassingDlg *sub = new SubclassingDlg(this, m_selectedUI, newFiles);
	if (sub->exec())
		project()->addFiles(newFiles);
	
	delete sub;
}

#include "kjssupport_part.moc"
