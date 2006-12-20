/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pythonsupportpart.h"
#include "pythonconfigwidget.h"

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevappfrontend.h>
#include <kdevpartcontroller.h>
#include <codemodel.h>
#include <domutil.h>

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qvbox.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <qregexp.h>

#include "qtdesignerpythonintegration.h"
#include "pythonimplementationwidget.h"

typedef KDevGenericFactory<PythonSupportPart> PythonSupportFactory;
static const KDevPluginInfo data("kdevpythonsupport");
K_EXPORT_COMPONENT_FACTORY( libkdevpythonsupport, PythonSupportFactory( data ) )

PythonSupportPart::PythonSupportPart(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport(&data, parent, name ? name : "PythonSupportPart")
{
    setInstance(PythonSupportFactory::instance());

    setXMLFile("kdevpythonsupport.rc");

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const KURL&)),
             this, SLOT(savedFile(const KURL&)) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
  connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
        this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

    KAction *action;

    action = new KAction( i18n("Execute Program"), "exec", 0,
                          this, SLOT(slotExecute()),
                          actionCollection(), "build_exec" );
    action->setToolTip( i18n("Execute program") );
    action->setWhatsThis(i18n("<b>Execute program</b><p>Runs the Python program."));

    action = new KAction( i18n("Execute String..."), "exec", 0,
                          this, SLOT(slotExecuteString()),
                          actionCollection(), "build_execstring" );
    action->setToolTip( i18n("Execute string") );
    action->setWhatsThis(i18n("<b>Execute String</b><p>Executes a string as Python code."));

    action = new KAction( i18n("Start Python Interpreter"), "exec", 0,
                          this, SLOT(slotStartInterpreter()),
                          actionCollection(), "build_runinterpreter" );
    action->setToolTip( i18n("Start Python interpreter") );
    action->setWhatsThis(i18n("<b>Start python interpreter</b><p>Starts the Python interpreter without a program"));

    action = new KAction( i18n("Python Documentation..."), 0,
                          this, SLOT(slotPydoc()),
                          actionCollection(), "help_pydoc" );
    action->setToolTip( i18n("Python documentation") );
    action->setWhatsThis(i18n("<b>Python documentation</b><p>Shows a Python documentation page."));
}


PythonSupportPart::~PythonSupportPart()
{}


void PythonSupportPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (context->hasType(Context::FileContext)){
        const FileContext *fc = static_cast<const FileContext*>(context);
        //this is a .ui file and only selection contains only one such file
        KURL url = fc->urls().first();
        if (url.fileName().endsWith(".ui"))
        {
            m_contextFileName = url.fileName();
            int id = popup->insertItem(i18n("Create or Select Implementation..."), this, SLOT(slotCreateSubclass()));
            popup->setWhatsThis(id, i18n("<b>Create or select implementation</b><p>Creates or selects a subclass of selected form for use with integrated KDevDesigner."));
        }
    }
}


void PythonSupportPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Python"));
    PythonConfigWidget *w = new PythonConfigWidget(*projectDom(), vbox, "python config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


void PythonSupportPart::projectOpened()
{
    kdDebug(9014) << "projectOpened()" << endl;

    connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
             this, SLOT(addedFilesToProject(const QStringList &)) );
    connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
             this, SLOT(removedFilesFromProject(const QStringList &)) );

    // We want to parse only after all components have been
    // properly initialized
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void PythonSupportPart::projectClosed()
{
}


void PythonSupportPart::maybeParse(const QString fileName)
{
    QFileInfo fi(fileName);
    if (fi.extension() == "py") {

	if( codeModel()->hasFile(fileName) ){
	    emit aboutToRemoveSourceInfo( fileName );
	    codeModel()->removeFile( codeModel()->fileByName(fileName) );
	}

        parse( fileName );
    }
}


void PythonSupportPart::initialParse()
{
    kdDebug(9014) << "initialParse()" << endl;

    if (project()) {
        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
            kdDebug(9014) << "maybe parse " << project()->projectDirectory() + "/" + (*it) << endl;
            maybeParse(project()->projectDirectory() + "/" + *it);
        }

        emit updatedSourceInfo();
        kapp->restoreOverrideCursor();
    } else {
        kdDebug(9014) << "No project" << endl;
    }
}


void PythonSupportPart::addedFilesToProject(const QStringList &fileList)
{
    kdDebug(9014) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		QString fileName = project()->projectDirectory() + "/" + ( *it );
		maybeParse( fileName );
		emit addedSourceInfo( fileName );
	}

    //emit updatedSourceInfo();
}


void PythonSupportPart::removedFilesFromProject(const QStringList &fileList)
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


void PythonSupportPart::savedFile(const KURL &fileName)
{
    kdDebug(9014) << "savedFile()" << endl;

    if (project()->allFiles().contains(fileName.path().mid ( project()->projectDirectory().length() + 1 ))) {
        maybeParse(fileName.path());
	emit addedSourceInfo( fileName.path() );
    }
}


KDevLanguageSupport::Features PythonSupportPart::features()
{
    return Features(Classes | Functions);
}


KMimeType::List PythonSupportPart::mimeTypes( )
{
    KMimeType::List list;
    KMimeType::Ptr mime = KMimeType::mimeType( "text/x-python" );
    if( mime )
	list << mime;

    mime = KMimeType::mimeType( "application/x-python" );
    if( mime )
        list << mime;

    return list;
}

void PythonSupportPart::parse(const QString &fileName)
{
    QFile f(QFile::encodeName(fileName));
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream stream(&f);

    QRegExp classre("^[ \t]*class[ \t]+([A-Za-z0-9_]+)[ \t]*(\\(([A-Za-z0-9_, \t]+)\\))?.*$");
    QRegExp methodre("^[ \t]*def[ \t]+([A-Za-z0-9_]+).*$");

    FileDom m_file = codeModel()->create<FileModel>();
    m_file->setName( fileName );

    ClassDom lastClass;
    QString rawline;
    QCString line;
    int lineNo = 0;
    while (!stream.atEnd()) {
        rawline = stream.readLine();
        line = rawline.stripWhiteSpace().local8Bit();
        if (classre.search(line) != -1) {

            lastClass = codeModel()->create<ClassModel>();
            lastClass->setName(classre.cap(1));
	    lastClass->setFileName( fileName );
	    lastClass->setStartPosition( lineNo, 0 );

            QStringList parentList = QStringList::split(",", classre.cap(3));
            QStringList::ConstIterator it;
            for (it = parentList.begin(); it != parentList.end(); ++it) {
                QString baseName = (*it).stripWhiteSpace();
                kdDebug(9014) << "Add parent" << baseName << endl;
                lastClass->addBaseClass( baseName );
            }

           if (m_file->hasClass(lastClass->name())) {
                ClassDom old = m_file->classByName( lastClass->name() )[ 0 ];
		old->setFileName( lastClass->fileName() );

		int line, col;
		lastClass->getStartPosition( &line, &col );
		old->setStartPosition( line, col );

                lastClass = old;
            } else {
                kdDebug(9014) << "Add class " << lastClass->name() << endl;
                m_file->addClass( lastClass );
            }

        } else if (methodre.search(line) != -1 ) {

            FunctionDom method = codeModel()->create<FunctionModel>();
            method->setName(methodre.cap(1));
	    method->setFileName( fileName );
            method->setStartPosition( lineNo, 0 );

            if (lastClass && rawline.left(3) != "def") {
	        if( !lastClass->hasFunction(method->name()) )
                    lastClass->addFunction( method );
                QStringList scope;
                scope << lastClass->name();
                method->setScope( scope );

            } else if( !m_file->hasFunction(method->name()) ){
                m_file->addFunction( method );
                lastClass = 0;
            }
        }
        ++lineNo;
    }

    f.close();

    codeModel()->addFile( m_file );
}


QString PythonSupportPart::interpreter()
{
    QString prog = DomUtil::readEntry(*projectDom(), "/kdevpythonsupport/run/interpreter");
    if (prog.isEmpty())
        prog = "python";

    return prog;
}


void PythonSupportPart::startApplication(const QString &program)
{
    bool inTerminal = DomUtil::readBoolEntry(*projectDom(), "/kdevpythonsupport/run/terminal");
    if (KDevAppFrontend *appFrontend = extension<KDevAppFrontend>("KDevelop/AppFrontend"))
        appFrontend->startAppCommand(QString::QString(), program, inTerminal);
}


void PythonSupportPart::slotExecute()
{
    QString program = project()->mainProgram();
    QString cmd = interpreter() + " " + program;
    startApplication(cmd);
}


void PythonSupportPart::slotStartInterpreter()
{
    startApplication(interpreter());
}


void PythonSupportPart::slotExecuteString()
{
    bool ok;
    QString cmd = KInputDialog::getText(i18n("String to Execute"), i18n("String to execute:"), QString::null, &ok, 0);
    if (ok) {
        cmd.prepend("'");
        cmd.append("'");
        startApplication(cmd);
    }
}


void PythonSupportPart::slotPydoc()
{
    bool ok;
    QString key = KInputDialog::getText(i18n("Show Python Documentation"), i18n("Show Python documentation on keyword:"), "", &ok, 0);
    if (ok && !key.isEmpty()) {
        QString url = "pydoc:";
        url += key;
        partController()->showDocument(KURL(url));
    }
}

KDevDesignerIntegration *PythonSupportPart::designer(KInterfaceDesigner::DesignerType type)
{
    KDevDesignerIntegration *des = 0;
    switch (type)
    {
        case KInterfaceDesigner::QtDesigner:
            des = m_designers[type];
            if (des == 0)
            {
                PythonImplementationWidget *impl = new PythonImplementationWidget(this);
                des = new QtDesignerPythonIntegration(this, impl);
                des->loadSettings(*project()->projectDom(),
                    "kdevpythonsupport/designerintegration");
                m_designers[type] = des;
            }
            break;
        case KInterfaceDesigner::Glade:
		    break;
    }
    return des;

}

void PythonSupportPart::slotCreateSubclass()
{
    QFileInfo fi(m_contextFileName);
    kdDebug(9014) << k_funcinfo << " file: " << m_contextFileName << " ext: " << fi.extension(false) << endl;
    if (fi.extension(false) != "ui")
        return;
    QtDesignerPythonIntegration *des = dynamic_cast<QtDesignerPythonIntegration*>(designer(KInterfaceDesigner::QtDesigner));
    if (des)
    {
        kdDebug(9014) << "ok: " << des << endl;
        kdDebug(9014) << "have impl: " << des->selectImplementation(m_contextFileName);
    }
    kdDebug(9014) << "end: " << des << endl;
}

#include "pythonsupportpart.moc"
