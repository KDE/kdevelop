/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qprogressdialog.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qstatusbar.h>
#include <qprogressbar.h>
#include <qregexp.h>
#include <qlabel.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kregexp.h>
#include <kmessagebox.h>

#include "keditor/editor.h"
#include "kdevcore.h"
#include "kdevproject.h"
#include "classstore.h"

#include "cppsupportpart.h"
#include "cppsupportfactory.h"
#include "parsedclass.h"
#include "parsedattribute.h"
#include "parsedmethod.h"
#include "classparser.h"
#include "addclassmethoddlg.h"
#include "addclassattributedlg.h"
#include "cppcodecompletion.h"

#include "keditor/edit_iface.h"

CppSupportPart::CppSupportPart(bool cpp, KDevApi *api, QObject *parent, const char *name)
    : KDevLanguageSupport(api, parent, name)
{
    setInstance(CppSupportFactory::instance());
    
    setXMLFile("kdevcppsupport.rc");

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( core(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( core()->editor(), SIGNAL(documentActivated(KEditor::Document*)),
             this, SLOT(documentActivated(KEditor::Document*)) );

    KAction *action;

    action = new KAction(i18n("Switch header/implementation"), Key_F12,
                         this, SLOT(slotSwitchHeader()),
                         actionCollection(), "edit_switchheader");
    action->setStatusText( i18n("Switch between header and implementation files") );
    action->setWhatsThis( i18n("Switch between header and implementation files\n\n"
                               "If you are currently looking at a header file, this "
                               "brings you to the corresponding implementation file. "
                               "If you are looking at an implementation file (.cpp etc.), "
                               "this brings you to the corresponding header file.") );
    action->setEnabled(false);

    m_pParser = 0;
    m_pCompletion = 0;
    m_pEditIface = 0;
    m_pCursorIface = 0;

    withcpp = cpp;
}


CppSupportPart::~CppSupportPart()
{
    delete m_pParser;
    delete m_pCompletion;
}


void CppSupportPart::documentActivated(KEditor::Document *doc)
{
    bool enabled = false;

    if (doc) {
        QFileInfo fi(doc->url().path());
        QString ext = fi.extension();
        ;
        if (QStringList::split(',', "c,cc,cpp,cxx,C,h,hxx").contains(ext))
            enabled = true;
    }

    actionCollection()->action("edit_switchheader")->setEnabled(enabled);

	m_pEditIface = KEditor::EditDocumentIface::interface(doc);
/*	disconnect(m_pEditIface, 0, this, 0 ); // to make sure that it is't connected twice
	connect(m_pEditIface,SIGNAL(textChanged()),
			m_pCompletion,SLOT(slotTextChanged()));*/

}


void CppSupportPart::projectOpened()
{
    kdDebug(9007) << "projectOpened()" << endl;

    connect( project(), SIGNAL(addedFileToProject(const QString &)),
             this, SLOT(addedFileToProject(const QString &)) );
    connect( project(), SIGNAL(removedFileFromProject(const QString &)),
             this, SLOT(removedFileFromProject(const QString &)) );

    // We want to parse only after all components have been
    // properly initialized
    m_pParser = new CClassParser( classStore());
    m_pCompletion = new CppCodeCompletion ( core(), classStore() );

    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void CppSupportPart::projectClosed()
{
	if ( !QFile::exists ( ( project()->projectDirectory() + "/" + project()->projectName() + ".pcs" ) ) )
	{
		if ( classStore()->open ( ( project()->projectDirectory() + "/" + project()->projectName() + ".pcs" ), IO_WriteOnly ) )
		{
			classStore()->storeAll();
			classStore()->close();
		}
	}

	delete m_pParser;
    delete m_pCompletion;

    m_pParser = 0;
    m_pCompletion = 0;
}


static QString findHeader(const QStringList &list, const QString &header)
{
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QString s = *it;
        int pos = s.findRev('.');
        if (pos != -1)
            s = s.left(pos) + ".h";
        if (s.right(header.length()) == header)
            return s;
    }

    return QString::null;
}


void CppSupportPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (context->hasType("editor")) {
        const EditorContext *econtext = static_cast<const EditorContext*>(context);
        QString str = econtext->linestr();
        //        int col = econtext->col();
        //        KRegExp re("[ \t]*#include[ \t]*[<\"](.*)[>\"][ \t]*");
        KRegExp re(" *#include *[<\"](.*)[>\"] *");
        if (re.match(str.latin1()) &&
            !findHeader(project()->allSourceFiles(), re.group(1)).isEmpty()) {
            popupstr = re.group(1);
            popup->insertItem( i18n("Goto include file: %1").arg(popupstr),
                               this, SLOT(slotGotoIncludeFile()) );
        }
    }
}


void CppSupportPart::maybeParse(const QString fileName)
{
    QFileInfo fi(fileName);
    QString path = fi.filePath();
    QString ext = fi.extension();
    if (ext == "cpp" || ext == "cc" || ext == "cxx") {
        QString headerFileName = path.left(path.length()-ext.length()) + "h";
        classStore()->removeWithReferences(headerFileName);
        m_pParser->parse(headerFileName);
        classStore()->removeWithReferences(fileName);
        m_pParser->parse(fileName);
    } else if (ext == "h") {
        classStore()->removeWithReferences(fileName);
        m_pParser->parse(fileName);
    }
}


void CppSupportPart::initialParse()
{
    kdDebug(9007) << "initialParse()" << endl;
    
    if (project())
	{
		if ( QFile::exists ( ( project()->projectDirectory() + "/" + project()->projectName() + ".pcs" ) ) )
		{
			if ( classStore()->open ( ( project()->projectDirectory() + "/" + project()->projectName() + ".pcs" ), IO_ReadOnly ) )
			{
				kdDebug ( 9007 ) << "loading persistant class store: " << ( project()->projectDirectory() + "/" + project()->projectName() + ".pcs" ) << endl;

				kapp->processEvents();
				kapp->setOverrideCursor(waitCursor);

				QLabel* label = new QLabel ( i18n ( "Loading class store..." ), core()->statusBar() );
				//label->setMinimumWidth ( 120 );

				core()->statusBar()->addWidget ( label );
				label->show();

				classStore()->restoreAll();
				classStore()->close();

				core()->statusBar()->removeWidget ( label );

				delete label;

				emit updatedSourceInfo();

				kapp->restoreOverrideCursor();
			}
		}
		else
		{
			kapp->setOverrideCursor(waitCursor);

			QStringList files = project()->allSourceFiles();

			int n = 0;
			QProgressBar *bar = new QProgressBar(files.count(), core()->statusBar());
			bar->setMinimumWidth(120);
			bar->setCenterIndicator(true);
			core()->statusBar()->addWidget(bar);
			bar->show();

			for (QStringList::Iterator it = files.begin(); it != files.end() ;++it)
			{
				bar->setProgress(n);
				kapp->processEvents();
				maybeParse(*it);
				++n;
			}

			core()->statusBar()->removeWidget(bar);
			delete bar;

			emit updatedSourceInfo();
			kapp->restoreOverrideCursor();
		}
	}
	else
	{
		kdDebug(9007) << "No project" << endl;
	}
}


void CppSupportPart::addedFileToProject(const QString &fileName)
{
    kdDebug(9007) << "addedFileToProject()" << endl;
    maybeParse(fileName);
    emit updatedSourceInfo();
}


void CppSupportPart::removedFileFromProject(const QString &fileName)
{
    kdDebug(9007) << "removedFileFromProject()" << endl;
    classStore()->removeWithReferences(fileName);
    emit updatedSourceInfo();
}


void CppSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9007) << "savedFile()" << endl;

    if (project()->allSourceFiles().contains(fileName)) {
        maybeParse(fileName);
        emit updatedSourceInfo();
    }
}


void CppSupportPart::slotSwitchHeader()
{
    QFileInfo fi(core()->editor()->currentDocument()->url().path());
    QString path = fi.filePath();
    QString ext = fi.extension();
    QString base = path.left(path.length()-ext.length());
    kdDebug(9007) << "base: " << base << ", ext: " << ext << endl;
    QStringList candidates;
    if (ext == "h" || ext == "hxx") {
        candidates << (base + "c");
        candidates << (base + "cc");
        candidates << (base + "cpp");
        candidates << (base + "cxx");
        candidates << (base + "C");
    } else if (QStringList::split(',', "c,cc,cpp,cxx,C").contains(ext)) {
        candidates << (base + "h");
        candidates << (base + "hxx");
    }
    
    QStringList::ConstIterator it;
    for (it = candidates.begin(); it != candidates.end(); ++it) {
        kdDebug(9007) << "Trying " << (*it) << endl;
        if (QFileInfo(*it).exists()) {
            core()->gotoSourceFile(*it);
            return;
        }
    }
}


void CppSupportPart::slotGotoIncludeFile()
{
    QString fileName = findHeader(project()->allSourceFiles(), popupstr);
    if (!fileName.isEmpty())
        core()->gotoSourceFile(fileName, 0);
    
}


KDevLanguageSupport::Features CppSupportPart::features()
{
    if (withcpp)
        return Features(Classes | Structs | Functions | Variables | Namespaces | Declarations
                        | Signals | Slots | AddMethod | AddAttribute);
    else
        return Features (Structs | Functions | Variables | Declarations);
}


void CppSupportPart::newClass()
{
}


void CppSupportPart::addMethod(const QString &className)
{
   if (! m_pEditIface) {
      KMessageBox::sorry(0, i18n("cann't get Interface: EditDocumentIface\nis the file open ?"), "OOPS" );
      return;
   }

    AddClassMethodDialog dlg( m_pParser->getClassStore(), className, 0, "methodDlg"); //TODO: Leak ?
    if (!dlg.exec())
        return;
    
    ParsedMethod *pm = dlg.asSystemObj();
    pm->setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass *pc = classStore()->getClassByName(className);
    
    if (pm->isSignal()) {
        for (pc->signalIterator.toFirst(); pc->signalIterator.current(); ++pc->signalIterator) {
            ParsedMethod *meth = pc->signalIterator.current();
            if (meth->access() == pm->access() &&
                atLine < meth->declarationEndsOnLine())
                atLine = meth->declarationEndsOnLine();
        }
    } else if (pm->isSlot()) {
        for (pc->slotIterator.toFirst(); pc->slotIterator.current(); ++pc->slotIterator) {
            ParsedMethod *meth = pc->slotIterator.current();
            if (meth->access() == pm->access() &&
                atLine < meth->declarationEndsOnLine())
                atLine = meth->declarationEndsOnLine();
        }
    } else {
        for (pc->methodIterator.toFirst(); pc->methodIterator.current(); ++pc->methodIterator) {
            ParsedMethod *meth = pc->methodIterator.current();
            if (meth->access() == pm->access() &&
                atLine < meth->declarationEndsOnLine())
                atLine = meth->declarationEndsOnLine();
        }
    }

    QString headerCode = asHeaderCode(pm);
    
    if (atLine == -1) {
        if (pm->isSignal())
            headerCode.prepend(QString("signals:\n"));
        else if (pm->access() == PIE_PUBLIC)
            headerCode.prepend(QString("public:%1\n").arg(pm->isSlot()? " slots" :  ""));
        else if (pm->access() == PIE_PROTECTED)
            headerCode.prepend(QString("protected:\n").arg(pm->isSlot()? " slots" :  ""));
        else if (pm->access() == PIE_PRIVATE)
            headerCode.prepend(QString("private:\n").arg(pm->isSlot()? " slots" :  ""));
        else
            kdDebug(9007) << "selectedAddMethod: Unknown access "
                          << (int)pm->access() << endl;

        atLine = pc->declarationEndsOnLine();
    } else 
        atLine++;

    core()->gotoSourceFile(pc->declaredInFile(), atLine);
    kdDebug() << "Adding to .h: " << atLine << " " << headerCode << endl;

    m_pEditIface->insertLine(headerCode, atLine);

    QString cppCode = asCppCode(pm);
    
    core()->gotoSourceFile(pc->definedInFile(), atLine);
    kdDebug() << "Adding to .cpp: " << atLine << " " << cppCode << endl;
    
    m_pEditIface->insertLine(cppCode, atLine);
    delete pm;
}


void CppSupportPart::addAttribute(const QString &className)
{
   if (! m_pEditIface) {
      KMessageBox::sorry(0, i18n("cann't get Interface: EditDocumentIface\nis the file open ?"), "OOPS" );
      return;
   }
    AddClassAttributeDialog dlg(0, "attrDlg");
    if( !dlg.exec() )
      return;

    ParsedAttribute *pa = dlg.asSystemObj();
    pa->setDeclaredInScope(className);

    int atLine = -1;
    ParsedClass *pc = classStore()->getClassByName(className);
    
    for (pc->attributeIterator.toFirst(); pc->attributeIterator.current(); ++pc->attributeIterator) {
        ParsedAttribute *attr = pc->attributeIterator.current();
        if (attr->access() == pa->access() &&
            atLine < attr->declarationEndsOnLine())
            atLine = attr->declarationEndsOnLine();
    }
    
    QString headerCode = asHeaderCode(pa);
    
    if (atLine == -1) {
        if (pa->access() == PIE_PUBLIC)
            headerCode.prepend("public: // Public attributes\n");
        else if (pa->access() == PIE_PROTECTED)
            headerCode.prepend("protected: // Protected attributes\n");
        else if (pa->access() == PIE_PRIVATE)
            headerCode.prepend("private: // Private attributes\n");
        else
            kdDebug(9007) << "selectedAddAttribute: Unknown access "
                          << (int)pa->access() << endl;

        atLine = pc->declarationEndsOnLine();
    } else 
        atLine++;

    core()->gotoSourceFile(pc->declaredInFile(), atLine);
    kdDebug() << "Adding at line " << atLine << " " << headerCode << endl;
    m_pEditIface->insertLine(headerCode, atLine);

    delete pa;
}


QString CppSupportPart::asHeaderCode(ParsedMethod *pm)
{
    QString str = "  ";
    str += pm->comment();
    str += "\n  ";

    if (pm->isVirtual())
        str += "virtual ";
    
    if (pm->isStatic())
        str += "static ";
    
    str += pm->type();
    str += " ";
    str += pm->name();
    
    if (pm->isConst())
        str += " const";
    
    if (pm->isPure())
        str += " = 0";
    
    str += ";\n";

    return str;
}


QString CppSupportPart::asCppCode(ParsedMethod *pm)
{
    if (pm->isPure() || pm->isSignal())
        return QString();

    QString str = pm->comment();
    str += "\n";

    // Take the path and replace all . with ::
    QString path = pm->path();
    path.replace( QRegExp( "\\." ), "::" );

    str += pm->type();
    str += " ";
    str += path;
    
    if (pm->isConst())
        str += " const";
    
    str += "{\n}\n";

    return str;
}


QString CppSupportPart::asHeaderCode(ParsedAttribute *pa)
{
    QString str = "  ";
    str += pa->comment();
    str += "\n  ";

    if (pa->isConst())
        str += "const ";

    if (pa->isStatic())
        str += "static ";

    str += pa->asString();
    str += ";\n";

    return str;
}

#include "cppsupportpart.moc"
