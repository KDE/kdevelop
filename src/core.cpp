/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>
#include <qdir.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <dcopclient.h>
#include <kapp.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/partmanager.h>
#include <kprocess.h>
#include <krun.h>
#include <kstdaction.h>
#include <kstddirs.h>
#include <ktrader.h>

#include "texteditor.h"
#include "classstore.h"
#include "kdevapi.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevproject.h"
#include "kdevlanguagesupport.h"
#include "KDevCoreIface.h"

#include "documentationpart.h"

#ifdef NEW_EDITOR
#include "keditor/editor.h"
#include "keditor/cursor_iface.h"
#else
#include "editorpart.h"
#endif

#include "filenameedit.h"
#include "importdlg.h"
#include "toplevel.h"
#include "partloader.h"
#include "core.h"


Core::Core()
    : KDevCore()
#ifdef NEW_EDITOR
      , _editor(0)
#endif
{
    api = new KDevApi();
    api->core = this;
    api->classStore = new ClassStore();
    api->document = 0;
    api->project = 0;

    win = new TopLevel();
    kapp->setMainWidget(win);

    dcopIface = new KDevCoreIface(this);
    //    kapp->dcopClient()->attach();
    kapp->dcopClient()->registerAs("gideon");

    connect( win, SIGNAL(wantsToQuit()),
             this, SLOT(wantsToQuit()) );

    activePart = 0;
    manager = new KParts::PartManager(win);
    connect( manager, SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(activePartChanged(KParts::Part*)) );

    initActions();
    win->createGUI(0);

#ifndef NEW_EDITOR
    bufferActions.setAutoDelete(true);
#endif 

#ifdef NEW_EDITOR
    // create editor object
    (void) editor();
#endif
    
    initComponents(); 

    win->show();

    emit coreInitialized();
}


Core::~Core()
{
    delete dcopIface;
}


KActionCollection *Core::actionCollection()
{
    return win->actionCollection();
}


void Core::initActions()
{
#ifndef NEW_EDITOR
    KStdAction::saveAs(this, SLOT(slotSaveFileAs()), actionCollection());
#endif
	KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

    KAction *action;
    
    action = new KAction( i18n("&Open file..."), "fileopen", CTRL+Key_O,
                          this, SLOT(slotOpenFile()),
                          actionCollection(), "file_open" );
#ifndef NEW_EDITOR
    action = new KAction( i18n("&Save file"), "filesave", 0,
                          this, SLOT(slotSaveFile()),
                          actionCollection(), "file_save" );

    action = new KAction( i18n("Split window &vertically"), 0,
                          this, SLOT(slotSplitVertically()),
                          actionCollection(), "file_splitvertically" );

    action = new KAction( i18n("Split window &horizontally"), 0,
                          this, SLOT(slotSplitHorizontally()),
                          actionCollection(), "file_splithorizontally" );
#endif

    action = new KAction( i18n("&Close window"), Key_F4,
                          this, SLOT(slotCloseWindow()),
                          actionCollection(), "file_closewindow" );

    action = new KAction( i18n("&Kill buffer"), 0,
                          this, SLOT(slotKillBuffer()),
                          actionCollection(), "file_killbuffer" );
    
    action = new KAction( i18n("&Open project..."), "project_open", 0,
                          this, SLOT(slotProjectOpen()),
                          actionCollection(), "project_open" );
    action->setStatusText( i18n("Opens a project") );

    action = new KAction( i18n("C&lose project"), "fileclose",0,
                          this, SLOT(slotProjectClose()),
                          actionCollection(), "project_close" );
    action->setEnabled(false);
    action->setStatusText( i18n("Closes the current project") );

    action = new KAction( i18n("&Import existing directory..."),"wizard", 0,
                          this, SLOT(slotProjectImport()),
                          actionCollection(), "project_import" );
    action->setStatusText( i18n("Creates a project file for a given directory.") );

    action = new KAction( i18n("&Customize Gideon"), 0,
                          this, SLOT(slotSettingsCustomize()),
                          actionCollection(), "settings_customize" );
    action->setStatusText( i18n("Lets you customize Gideon") );

    action = new KAction( i18n("&Stop"), "stop", 0,
                          this, SLOT(slotStop()),
                          actionCollection(), "stop_processes");
    action->setStatusText( i18n("Stops all running subprocesses") );
    action->setEnabled(false);   
}


void Core::initComponent(KDevPart *part)
{
    kdDebug(9000) << "Init " << part->name() << endl;
    components.append(part);
    win->guiFactory()->addClient(part);
}


void Core::removeComponent(KDevPart *part)
{
    kdDebug(9000) << "Removing " << part->name() << endl;
    win->guiFactory()->removeClient(part);
    components.remove(part);
    delete part;
}


void Core::initComponents()
{
    // These two parts are compiled in, so we don't have to check loadByQuery's return value
    KDevPart *makeFrontend =
        PartLoader::loadByQuery(QString::fromLatin1("KDevelop/MakeFrontend"), QString::null, "KDevMakeFrontend",
                                api, this);
    initComponent(api->makeFrontend = static_cast<KDevMakeFrontend*>(makeFrontend));
    KDevPart *appFrontend =
        PartLoader::loadByQuery(QString::fromLatin1("KDevelop/AppFrontend"), QString::null, "KDevAppFrontend",
                                api, this);
    initComponent(api->appFrontend = static_cast<KDevAppFrontend*>(appFrontend));

    QList<KDevPart> parts =
        PartLoader::loadAllByQuery(QString::fromLatin1("KDevelop/Part"), QString::null, "KDevPart",
                                   api, this, true);

    QListIterator<KDevPart> it(parts);
    for (; it.current(); ++it)
        initComponent(*it);
}


void Core::removeComponents()
{
    QListIterator<KDevPart> it(components);
    for (; it.current(); ++it) {
        kdDebug(9000) << "Still have part " << (*it)->name() << endl;
    }
        
    while (!components.isEmpty())
        removeComponent(components.first());
}


DocumentationPart *Core::createDocumentationPart()
{
    DocumentationPart *part = new DocumentationPart(win);
    manager->addPart(part, false);
    manager->addManagedTopLevelWidget(part->widget());
    docParts.append(part);
    connect( part, SIGNAL(destroyed()),
             this, SLOT(docPartDestroyed()));
    connect( part, SIGNAL(contextMenu(QPopupMenu *, const QString &, const QString &)),
             this, SLOT(docContextMenu(QPopupMenu *, const QString &, const QString &)) );
    connect( part, SIGNAL(setStatusBarText(const QString&)),
             win->statusBar(), SLOT(message(const QString&)) );
    return part;
}


void Core::activePartChanged(KParts::Part *part)
{
    win->createGUI(part);
    activePart = part;

	// TODO: enable/disable actions depending on the current part!
    	
#ifndef NEW_EDITOR
    if (activePart && activePart->inherits("EditorPart")) {
        EditorPart *part = static_cast<EditorPart*>(activePart);
        TextEditorDocument *doc = part->editorDocument();
        if (doc->modifiedOnDisk() && KMessageBox::warningYesNo
            (win, i18n("The file %1 was modified on disk.\n"
                       "Revert to the version on disk now?").arg(doc->url().url()))) {
            doc->openURL(doc->url());
        }
    }
#endif
}


void Core::docPartDestroyed()
{
    DocumentationPart *part = (DocumentationPart*) sender();
    docParts.removeRef(part);
}


void Core::docContextMenu(QPopupMenu *popup, const QString &url, const QString &selection)
{
    DocumentationContext context(url, selection);
    emit contextMenu(popup, &context);
}

#ifdef NEW_EDITOR

KEditor::Editor *Core::editor()
{
  if (_editor)
    return _editor;

  // ask the trader which editor he has to offer
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KDevelop/Editor"), QString::null);
  if (offers.isEmpty())
        return 0;

  // for now, pick the first one
  KService *service = *offers.begin();
  KLibFactory *factory = KLibLoader::self()->factory(service->library());
  if (!factory)
        return 0;

  // cast it, so we can easily access the part later
  _editor = static_cast<KEditor::Editor*>(factory->create(win, "editor"));
  if (!_editor)
        return 0;

  // merge the GUI with ours
  win->factory()->addClient(_editor);

  // make the editor aware of part activations
  connect(partManager(), SIGNAL(activePartChanged(KParts::Part*)), _editor, SLOT(activePartChanged(KParts::Part*)));

  return _editor;
}

#else

EditorPart *Core::createEditorPart()
{
    EditorPart *part = new EditorPart(win);
    manager->addPart(part, false);
    manager->addManagedTopLevelWidget(part->widget());
    editorParts.append(part);
    connect( part, SIGNAL(destroyed()),
             this, SLOT(editorPartDestroyed()));
    connect( part, SIGNAL(contextMenu(QPopupMenu *, const QString &, int)),
             this, SLOT(editorContextMenu(QPopupMenu *, const QString &, int)) );
    connect( part, SIGNAL(setStatusBarText(const QString&)),
             win->statusBar(), SLOT(message(const QString&)) );
    connect( part, SIGNAL(wentToSourceFile(const QString &)),
             this, SIGNAL(wentToSourceFile(const QString &)));
    connect( part, SIGNAL(toggledBreakpoint(const QString &, int)),
             this, SIGNAL(toggledBreakpoint(const QString &, int)) );
    connect( part, SIGNAL(editedBreakpoint(const QString &, int)),
             this, SIGNAL(toggledBreakpoint(const QString &, int)) );
    connect( part, SIGNAL(toggledBreakpointEnabled(const QString &, int)),
             this, SIGNAL(toggledBreakpointEnabled(const QString &, int)) );


    return part;
}
#endif


void Core::editorPartDestroyed()
{
#ifndef NEW_EDITOR
    kdDebug() << "editor part destroyed" << endl;
    EditorPart *part = (EditorPart*) sender();
    editorParts.removeRef(part);
#endif
}


void Core::editorContextMenu(QPopupMenu *popup, const QString &linestr, int col)
{
    EditorContext context(linestr, col);
    emit contextMenu(popup, &context);
}



void Core::updateBufferMenu()
{
#ifdef NEW_EDITOR
	
	QList<KAction> bufferActions;
	
	win->unplugActionList("buffer_list");
	
	QListIterator<KParts::Part> it(*partManager()->parts());
	for ( ; it.current(); ++it)
	{
	  kdDebug(9000) << "listing part: " << it.current()->className() << endl;
	  if (it.current()->inherits("KParts::ReadOnlyPart"))
	  {
		KParts::ReadOnlyPart *ro_part = static_cast<KParts::ReadOnlyPart*>(it.current());
		QString name = ro_part->url().url();
		if (name.isEmpty())
		    continue;
		kdDebug(9000) << "Plugging " << name << endl;
		KAction *action = new KAction(name, 0, 0, name.latin1());
		connect(action, SIGNAL(activated()), this, SLOT(slotBufferSelected()));
		bufferActions.append(action);
	  }
	}
	
	win->plugActionList("buffer_list", bufferActions);
	
#else
	win->unplugActionList("buffer_list");
    bufferActions.clear();

    QListIterator<TextEditorDocument> it(editedDocs);
    for (; it.current(); ++it) {
        QString fileName = it.current()->fileName();
        kdDebug(9000) << "Plugging " << fileName << endl;
        KAction *action = new KAction(fileName, 0, 0, fileName.latin1());
        connect( action, SIGNAL(activated()), this, SLOT(slotBufferSelected()) );
        bufferActions.append(action);
    }
    win->plugActionList("buffer_list", bufferActions);
#endif
}


void Core::closeProject()
{
    api->classStore->wipeout();
    if  (api->project) {
        emit projectClosed();
        api->project->closeProject();
        if (api->languageSupport) {
            removeComponent(api->languageSupport);
            api->languageSupport = 0;
        }
        if (api->document) {
            QFile fout(projectFile);
            if (fout.open(IO_WriteOnly)) {
                QTextStream stream(&fout);
                api->document->save(stream, 2);
            } else {
                KMessageBox::sorry(win, i18n("Could not write the project file."));
            }
            fout.close();
            delete api->document;
            api->document = 0;
        }
        removeComponent(api->project);
        api->project = 0;
    }

    projectFile = QString::null;
    win->setCaption(QString::fromLatin1(""));
    actionCollection()->action("project_close")->setEnabled(false);
}


void Core::openProject()
{
    QFile fin(projectFile);
    if (!fin.open(IO_ReadOnly)) {
        KMessageBox::sorry(win, "Could not read project file.");
        return;
    }

    QDomDocument *doc = new QDomDocument();
    if (!doc->setContent(&fin) || doc->doctype().name() != "kdevelop") {
        KMessageBox::sorry(win, "This is not a valid project file.");
        delete doc;
        fin.close();
        return;
    }
    api->document = doc;
    fin.close();

    QDomElement docEl = api->document->documentElement();
    QDomElement generalEl = docEl.namedItem("general").toElement();
    QDomElement projectEl = generalEl.namedItem("projectmanagement").toElement();
    QString projectPlugin = projectEl.firstChild().toText().data();
    kdDebug(9000) << "Project plugin: " << projectPlugin << endl;
    QDomElement primarylanguageEl = generalEl.namedItem("primarylanguage").toElement();
    QString language = primarylanguageEl.firstChild().toText().data();
    kdDebug(9000) << "Primary language: " << language << endl;
    
    KDevPart *project =
        PartLoader::loadByName(projectPlugin, "KDevProject",
                               api, this);
    if (project)
        initComponent(api->project = static_cast<KDevProject*>(project));
    else
        KMessageBox::sorry(win, i18n("No project management plugin %1 found.").arg(projectPlugin));
    
    KDevPart *languageSupport =
        PartLoader::loadByQuery(QString::fromLatin1("KDevelop/LanguageSupport"),
                                QString::fromLatin1("[X-KDevelop-Language] == '%1'").arg(language),
                                "KDevLanguageSupport",
                                api, this);
    if (languageSupport)
        initComponent(api->languageSupport = static_cast<KDevLanguageSupport*>(languageSupport));
    else
        KMessageBox::sorry(win, i18n("No language plugin for %1 found.").arg(language));

    QFileInfo fi(projectFile);
    QString projectDir = fi.dirPath();
    kdDebug(9000) << "projectDir: " << projectDir << endl;
    
    if (api->project)
        api->project->openProject(projectDir);
    emit projectOpened();
    
    win->setCaption(projectDir);
    actionCollection()->action("project_close")->setEnabled(true);
}


void Core::embedWidget(QWidget *w, KDevCore::Role role, const QString &shortCaption)
{
    (void) win->embedToolWidget(w, role, shortCaption);
}


void Core::raiseWidget(QWidget *w)
{
    win->raiseWidget(w);
}


void Core::gotoFile(const KURL &url)
{
    QString mimeType = KMimeType::findByURL(url)->name();
    if (!mimeType.startsWith("text/")) {
        new KRun(url);
    } else {
        gotoSourceFile(url, 0, Replace);
    }
}


void Core::gotoDocumentationFile(const KURL& url, Embedding embed)
{
    kdDebug(9000) << "Goto documentation: " << url.url() << endl;

    // Find a part to load into
    DocumentationPart *part = 0;
    if (embed == Replace && activePart && activePart->inherits("DocumentationPart"))
        part = static_cast<DocumentationPart*>(activePart);
    else {
        part = createDocumentationPart();
        if (embed == SplitHorizontal || embed == SplitVertical)
            win->splitDocumentWidget(part->widget(),
                                     activePart? activePart->widget() : 0,
                                     (embed==SplitHorizontal)? Horizontal : Vertical);
        else
            win->embedDocumentWidget(part->widget(),
                                     activePart? activePart->widget() : 0);
    }
    
    part->gotoURL(url);
    win->raiseWidget(part->widget());

	//TODO: fix the documentation part so that it behaves like a normal
	//part.
	//updateBufferMenu();
}


#ifdef NEW_EDITOR
KEditor::Document *Core::createDocument(const KURL &url)
{
  KEditor::Document *doc = editor()->createDocument(win, url);
  if (!doc)
      return 0;

  partManager()->addPart(doc);

  return doc;
}
#endif


void Core::gotoSourceFile(const KURL& url, int lineNum, Embedding embed)
{
    kdDebug(9000) << "Goto source file: " << url.path() << " line: " << lineNum << endl;

#ifdef NEW_EDITOR

	KParts::Part *activePart = partManager()->activePart();

    // activate the requested file
    KEditor::Document *doc = editor()->document(url);
    if (!doc)
	{
        doc = createDocument(url);
		updateBufferMenu();
	}
    if (!doc)
        return;

	if (activePart != doc)
	  partManager()->setActivePart(doc, doc->widget());

    // goto the requested line
    KEditor::CursorDocumentIface *iface = static_cast<KEditor::CursorDocumentIface*>(doc->queryInterface("KEditor::CursorDocumentIface"));
    if (iface)
        iface->setCursorPosition(lineNum, 0);

    if (!doc->widget())
      return;

	if (doc != activePart)
	{
      if (embed == SplitHorizontal || embed == SplitVertical)
        win->splitDocumentWidget(doc->widget(), activePart ? activePart->widget() : 0, (embed==SplitHorizontal) ? Horizontal : Vertical);
      else
        win->embedDocumentWidget(doc->widget(), activePart ? activePart->widget() : 0);
    }

  if (doc->widget())	
    win->raiseWidget(doc->widget());
  
#else

    // See if we have this document already loaded.
    TextEditorDocument *doc = 0;
    QListIterator<TextEditorDocument> it2(editedDocs);
    for (; it2.current(); ++it2) {
        if ((*it2)->isEditing(url)) {
            doc = it2.current();
            break;
        }
    }

    if (!doc) {
        kdDebug(9000) << "Doc not found, loading now" << endl;
        doc = new TextEditorDocument();
        doc->openURL(url);
        editedDocs.append(doc);
        updateBufferMenu();
        emit loadedFile(doc->fileName());
    }

    // Find a part to load into
    EditorPart *part = 0;
    if (embed == Replace && activePart && activePart->inherits("EditorPart")) {
        kdDebug(9000) << "Reusing editor part" << endl;
        part = static_cast<EditorPart*>(activePart);
    } else {
        kdDebug(9000) << "Creating new editor part" << endl;
        part = createEditorPart();
        if (embed == SplitHorizontal || embed == SplitVertical)
            win->splitDocumentWidget(part->widget(),
                                     activePart? activePart->widget() : 0,
                                     (embed==SplitHorizontal)? Horizontal : Vertical);
        else
            win->embedDocumentWidget(part->widget(),
                                     activePart? activePart->widget() : 0);
    }
    
    part->gotoDocument(doc, lineNum);
    win->raiseWidget(part->widget());
#endif
}


void Core::gotoExecutionPoint(const QString &fileName, int lineNum)
{
    KURL url(fileName);
#ifdef NEW_EDITOR

	gotoSourceFile(url, lineNum);

    // TODO: This should use the Breakpoint/Debug iface once it exists!
#else
    QListIterator<TextEditorDocument> it(editedDocs);
    for (; it.current(); ++it)
        (*it)->setExecutionPoint(-1);

    if (fileName.isEmpty())
        return;
    
    gotoSourceFile(url, lineNum);

    QListIterator<TextEditorDocument> it2(editedDocs);
    for (; it2.current(); ++it2)
        if ((*it2)->isEditing(url)) {
            (*it2)->setExecutionPoint(lineNum);
            break;
        }
#endif
}


void Core::saveAllFiles()
{
#ifdef NEW_EDITOR

    QListIterator<KParts::Part> it(*partManager()->parts());
	for ( ; it.current(); ++it)
	    if (it.current()->inherits("KParts::ReadWritePart"))
		{
			KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(it.current());
			rw_part->save();
		}
		
#else	
	QListIterator<TextEditorDocument> it(editedDocs);
    for (; it.current(); ++it) {
        TextEditorDocument *doc = it.current();
        if (doc->isModified() && doc->modifiedOnDisk()) {
            int res = KMessageBox::warningYesNoCancel
                (win, i18n("The file %1 was modified on the disk.\n"
                           "Save anyway?").arg(doc->url().url()));
            if (res == KMessageBox::Cancel)
                return;
            if (res == KMessageBox::Yes) {
                doc->save();
                win->statusBar()->message(i18n("Saved %1").arg(doc->fileName()));
                emit savedFile(doc->fileName());
            }
        }
    }
#endif
}


void Core::revertAllFiles()
{
#ifndef NEW_EDITOR
    QListIterator<TextEditorDocument> it(editedDocs);
    for (; it.current(); ++it) {
        TextEditorDocument *doc = it.current();
        if (doc->isModified()) {
            doc->openURL(doc->url());
            win->statusBar()->message(i18n("Reverted %1").arg(doc->fileName()));
            emit loadedFile(doc->fileName());
        }
    }
#endif
}


void Core::setBreakpoint(const QString &fileName, int lineNum,
                         int id, bool enabled, bool pending)
{
    KURL url(fileName);

#ifndef NEW_EDITOR

    QListIterator<TextEditorDocument> it(editedDocs);
    for (; it.current(); ++it) {
        if ((*it)->isEditing(url)) {
            (*it)->setBreakpoint(lineNum, id, enabled, pending);
            break;
        }
    }
#endif
}


void Core::running(KDevPart *part, bool runs)
{
    if (runs)
        runningComponents.append(part);
    else
        runningComponents.remove(part);

    actionCollection()->action("stop_processes")->setEnabled(!runningComponents.isEmpty());
}


void Core::message(const QString &str)
{
    win->statusBar()->message(str);
}


void Core::wantsToQuit()
{
    QTimer::singleShot(0, this, SLOT(slotQuit()));
}


void Core::openFileInteractionFinished(const QString &fileName)
{
    gotoSourceFile(KURL(fileName), 0);
}


void Core::saveFileInteractionFinished(const QString &fileName)
{
#ifndef NEW_EDITOR
    if (!activePart->inherits("EditorPart"))
        return;

    EditorPart *part = static_cast<EditorPart*>(activePart);
    TextEditorDocument *doc = part->editorDocument();

    QListIterator<TextEditorDocument> it(editedDocs);
    for (; it.current(); ++it)
        if (it.current() != doc && it.current()->isEditing(fileName)) {
            KMessageBox::sorry(win, i18n("This filename is already used by another buffer."));
            return;
        }

    QFileInfo fi(fileName);
    if (fi.exists()) {
        if (!KMessageBox::warningYesNo
            (win, i18n("A file with the name %1 already exists.\n"
                       "Overwrite it?").arg(fileName)))
            return;
    }

    doc->saveAs(KURL(fileName));
    part->updateWindowCaption();
    updateBufferMenu();
    win->statusBar()->message(i18n("Saved %1").arg(doc->fileName()));
    emit savedFile(doc->fileName());
#endif
}


void Core::slotOpenFile()
{
#if 1
    QString fileName = KFileDialog::getOpenFileName();
    openFileInteractionFinished(fileName);
#else
    // currently broken
    FileNameEdit *w = new FileNameEdit(i18n("Open file:"), win->statusBar());

    if (activePart && activePart->inherits("EditorPart")) {
        EditorPart *part = static_cast<EditorPart*>(activePart);
        TextEditorDocument *doc = part->editorDocument();
        QString dir = QFileInfo(doc->fileName()).dirPath();
        w->setText(dir);
    } else {
        w->setText(QDir::homeDirPath());
    }
    w->show();
    w->setFocus();

    connect( w, SIGNAL(finished(const QString &)),
             this, SLOT(openFileInteractionFinished(const QString &)) );
#endif
}


void Core::slotSaveFile()
{
    // FIXME: enable/disable this action for active DocumentionPart
    if (!activePart->inherits("EditorPart"))
        return;
#ifndef NEW_EDITOR
    EditorPart *part = static_cast<EditorPart*>(activePart);
    TextEditorDocument *doc = part->editorDocument();
    doc->save();
    win->statusBar()->message(i18n("Saved %1").arg(doc->fileName()));
    emit savedFile(doc->fileName());
#endif
}


void Core::slotSaveFileAs()
{
    
#if 1
    
    QString fileName = KFileDialog::getSaveFileName();
    saveFileInteractionFinished(fileName);
    
#else
    
    // currently broken
    // FIXME: enable/disable this action for active DocumentionPart
    if (!activePart->inherits("EditorPart"))
        return;

    FileNameEdit *w = new FileNameEdit(i18n("Save file as:"), win->statusBar());
    
    EditorPart *part = static_cast<EditorPart*>(activePart());
    TextEditorDocument *doc = part->editorDocument();
    QString fileName = doc->fileName();
    w->setText(fileName);
    w->show();
    w->setFocus();

    connect( w, SIGNAL(finished(const QString &)),
             this, SLOT(saveFileInteractionFinished(const QString &)) );
    
#endif
    
}


void Core::slotCloseWindow()
{
    if (!activePart)
        return;

#ifdef NEW_EDITOR
    delete activePart;

#else 
    if (activePart->inherits("EditorPart")) {
        EditorPart *part = static_cast<EditorPart*>(activePart);
        TextEditorDocument *doc = part->editorDocument();
        if (doc->isModified() && !KMessageBox::warningYesNo
            (win, i18n("The file %1 is modified.\n"
                       "Close this window anyway?").arg(doc->url().url())))
            return;
    }
    
    delete activePart->widget();
#endif
}


void Core::splitWindow(Orientation orient)
{
    if (!activePart)
        return;
#ifndef NEW_EDITOR
    if (activePart->inherits("EditorPart")) {
        EditorPart *part = static_cast<EditorPart*>(activePart);
        TextEditorDocument *doc = part->editorDocument();
        EditorPart *newpart = createEditorPart();
        win->splitDocumentWidget(newpart->widget(), activePart->widget(), orient);
        newpart->gotoDocument(doc, 0);
        win->raiseWidget(newpart->widget());
    } else if (activePart->inherits("DocumentationPart")) {
        DocumentationPart *part = static_cast<DocumentationPart*>(activePart);
        KURL url = part->browserURL();
        DocumentationPart *newpart = createDocumentationPart();
        win->splitDocumentWidget(newpart->widget(), activePart->widget(), orient);
        newpart->gotoURL(url);
        win->raiseWidget(newpart->widget());
    }
#endif
}


void Core::slotSplitVertically()
{
    splitWindow(Vertical);
}


void Core::slotSplitHorizontally()
{
    splitWindow(Horizontal);
}


void Core::slotKillBuffer()
{
#ifndef NEW_EDITOR
    if (!activePart)
        return;

    if (activePart->inherits("EditorPart")) {
        EditorPart *part = static_cast<EditorPart*>(activePart);
        TextEditorDocument *doc = part->editorDocument();
        if (doc->isModified() && !KMessageBox::warningYesNo
            (win, i18n("The file %1 is modified.\n"
                       "Close this window anyway?").arg(doc->url().url())))
            return;

        QList<EditorPart> deletedParts;

        QListIterator<EditorPart> it1(editorParts);
        for (; it1.current(); ++it1) {
            if (it1.current()->editorDocument() == doc)
                deletedParts.append(it1.current());
        }

        // Kill all parts that are still looking at this document
        // - One may want to change this to Emacs' behaviour
        QListIterator<EditorPart> it2(deletedParts);
        for (; it2.current(); ++it2)
            delete it2.current()->widget();

        editedDocs.remove(doc);
        updateBufferMenu();
        delete doc;
    } else if (activePart->inherits("DocumentationPart")) {
        // Of course, calling a documentation window a 'buffer'
        // is not really intuitive, but I currently have no
        // better idea...
        delete activePart->widget();
    }
#endif
}


void Core::slotQuit()
{
#ifndef NEW_EDITOR
    QListIterator<TextEditorDocument> it(editedDocs);
    for (; it.current(); ++it) {
        TextEditorDocument *doc = it.current();
        if (doc->isModified()) {
            int res = KMessageBox::warningYesNoCancel
                (win, i18n("The file %1 is modified.\n"
                           "Save this file now?").arg(doc->url().url()));
            if (res == KMessageBox::Cancel)
                return;
            if (res == KMessageBox::Yes)
                doc->save();
        }
    }
#endif
    disconnect( manager, SIGNAL(activePartChanged(KParts::Part*)),
                this, SLOT(activePartChanged(KParts::Part*)) );

    closeProject();
    removeComponents();

    win->closeReal();
}


void Core::slotProjectOpen()
{
    QString fileName = KFileDialog::getOpenFileName(QString::null, "*.kdevelop", win, i18n("Open project"));
    if (fileName.isNull())
      return;
    
    closeProject();
    projectFile = fileName;
    openProject();
}


void Core::slotProjectClose()
{
    closeProject();
}


void Core::slotProjectImport()
{
    ImportDialog *dlg = new ImportDialog(win, "import dialog");
    dlg->exec();
    delete dlg;
}


void Core::slotBufferSelected()
{
#ifdef NEW_EDITOR

	// Note: this code is intented to work with more than
 	// just editor parts, e.g. with documentation parts. There
	// are two problems right now: a) the documentation part's
	// implementation is broken, and b) we need a better mechanism
	// than the URL to identify the parts.
	
    QListIterator<KParts::Part> it(*partManager()->parts());
	for ( ; it.current(); ++it)
	{
	  if (!it.current()->inherits("KParts::ReadOnlyPart"))
		  continue;

	  KParts::ReadOnlyPart *ro_part = static_cast<KParts::ReadOnlyPart*>(it.current());
	  if (ro_part->url() == KURL(sender()->name()))
		{
		  KParts::Part *activePart = partManager()->activePart();

          partManager()->setActivePart(it.current(), it.current()->widget());
 
		  if (!it.current()->widget())
		      return;

          if (activePart != ro_part)		  
	        win->embedDocumentWidget(it.current()->widget(), activePart ? activePart->widget() : 0);
	
	      win->raiseWidget(it.current()->widget());	
		}
	  }
#else
    QString fileName = sender()->name();
    gotoSourceFile(KURL(fileName), 0);
#endif
}


void Core::slotSettingsCustomize()
{
    KDialogBase dlg(KDialogBase::TreeList, i18n("Customize Gideon"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, win,
                    "customization dialog");

    emit configWidget(&dlg);
    dlg.exec();
}


void Core::slotStop()
{
    // Hmm, not much to do ;-)
    emit stopButtonClicked();
}

#include "core.moc"
