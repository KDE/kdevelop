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

#include "documentationpart.h"
#include "editorpart.h"
#include "filenameedit.h"
#include "importdlg.h"
#include "toplevel.h"
#include "partloader.h"
#include "core.h"


Core::Core()
    : KDevCore()
{
    api = new KDevApi();
    api->core = this;
    api->classStore = new ClassStore();
    api->document = 0;
    api->project = 0;

    win = new TopLevel();
    kapp->setMainWidget(win);
    kapp->dcopClient();

    connect( win, SIGNAL(wantsToQuit()),
             this, SLOT(wantsToQuit()) );
    
    manager = new KParts::PartManager(win);
    connect( manager, SIGNAL(activePartChanged(KParts::Part*)),
             win, SLOT(createGUI(KParts::Part*)) );

    initActions();
    win->createGUI();

    bufferActions.setAutoDelete(true);
    
    //gotoSourceFile("/home/bernd/.emacs", 0);

    initComponents();
    win->show();
}


Core::~Core()
{}


KActionCollection *Core::actionCollection()
{
    return win->actionCollection();
}


void Core::initActions()
{
    KStdAction::saveAs(this, SLOT(slotSaveFileAs()), actionCollection());
    KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

    KAction *action;
    
    action = new KAction( i18n("&Open file..."), "fileopen", CTRL+Key_O,
                          this, SLOT(slotOpenFile()),
                          actionCollection(), "file_open" );

    action = new KAction( i18n("&Save file"), "filesave", 0,
                          this, SLOT(slotSaveFile()),
                          actionCollection(), "file_save" );

    action = new KAction( i18n("Split window &vertically"), 0,
                          this, SLOT(slotSplitVertically()),
                          actionCollection(), "file_splitvertically" );

    action = new KAction( i18n("Split window &horizontally"), 0,
                          this, SLOT(slotSplitHorizontally()),
                          actionCollection(), "file_splithorizontally" );

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

    action = new KAction( i18n("C&lose project"), 0,
                          this, SLOT(slotProjectClose()),
                          actionCollection(), "project_close" );
    action->setEnabled(false);
    action->setStatusText( i18n("Closes the current project") );

    action = new KAction( i18n("&Import existing directory..."), 0,
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
                                   api, this);

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
    manager->addPart(part, true);
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


EditorPart *Core::createEditorPart()
{
    EditorPart *part = new EditorPart(win);
    manager->addPart(part, true);
    manager->addManagedTopLevelWidget(part->widget());
    editorParts.append(part);
    connect( part, SIGNAL(destroyed()),
             this, SLOT(editorPartDestroyed()));
    connect( part, SIGNAL(contextMenu(QPopupMenu *, const QString &, int)),
             this, SLOT(editorContextMenu(QPopupMenu *, const QString &, int)) );
    connect( part, SIGNAL(setStatusBarText(const QString&)),
             win->statusBar(), SLOT(message(const QString&)) );
    return part;
}


void Core::editorPartDestroyed()
{
    kdDebug() << "editor part destroyed" << endl;
    EditorPart *part = (EditorPart*) sender();
    editorParts.removeRef(part);
}


void Core::editorContextMenu(QPopupMenu *popup, const QString &linestr, int col)
{
    EditorContext context(linestr, col);
    emit contextMenu(popup, &context);
}


void Core::updateBufferMenu()
{
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
            QFile fout(projectDir + "/gideonprj");
            if (fout.open(IO_WriteOnly)) {
                QTextStream stream(&fout);
                api->document->save(stream, 2);
            }
            fout.close();
            delete api->document;
            api->document = 0;
        }
        removeComponent(api->project);
        api->project = 0;
    }

    projectDir = QString::null;
    win->setCaption(QString::fromLatin1(""));
    actionCollection()->action("project_close")->setEnabled(false);
}


void Core::openProject()
{
    QFile fin(projectDir + "/gideonprj");
    if (fin.open(IO_ReadOnly)) {
        QDomDocument *doc = new QDomDocument();
        if (doc->setContent(&fin) && doc->doctype().name() == "gideon")
            api->document = doc;
        else
            delete doc;
        fin.close();
    }

    if (api->document) {
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
    }

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


void Core::gotoSourceFile(const KURL& url, int lineNum, Embedding embed)
{
    kdDebug(9000) << "Goto source file: " << url.path() << endl;
    QString fileName = url.path();
    
    // Find a part to load into
    EditorPart *part = 0;
    KParts::Part *activePart = manager->activePart();
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
    
    // See if we have this document already loaded.
    TextEditorDocument *doc = 0;
    QListIterator<TextEditorDocument> it2(editedDocs);
    for (; it2.current(); ++it2) {
        // TODO: Instead of comparing file names, use devno, inode
        if ((*it2)->fileName() == fileName) {
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
    }       

    part->gotoDocument(doc, lineNum);
    win->raiseWidget(part->widget());
}


void Core::gotoDocumentationFile(const KURL& url, Embedding embed)
{
    kdDebug(9000) << "Goto documentation: " << url.url() << endl;

    // Find a part to load into
    DocumentationPart *part = 0;
    KParts::Part *activePart = manager->activePart();

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
}


void Core::saveAllFiles()
{
    QListIterator<TextEditorDocument> it(editedDocs);
    for (; it.current(); ++it) {
        TextEditorDocument *doc = it.current();
        if (doc->isModified()) {
            doc->saveFile();
            win->statusBar()->message(i18n("Saved %1").arg(doc->fileName()));
            emit savedFile(doc->fileName());
        }
    }
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
    if (!manager->activePart()->inherits("EditorPart"))
        return;

    EditorPart *part = static_cast<EditorPart*>(manager->activePart());
    TextEditorDocument *doc = part->editorDocument();
    doc->setURL(KURL(fileName), false);
    doc->saveFile();
    win->statusBar()->message(i18n("Saved %1").arg(doc->fileName()));
    emit savedFile(doc->fileName());
}


void Core::slotOpenFile()
{
#if 0
    QString fileName = KFileDialog::getOpenFileName();
    gotoSourceFile(KURL(fileName), 0);
#else
    FileNameEdit *w = new FileNameEdit(i18n("Open file:"), win->statusBar());
    KParts::Part *activePart = manager->activePart();
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
    if (!manager->activePart()->inherits("EditorPart"))
        return;

    EditorPart *part = static_cast<EditorPart*>(manager->activePart());
    TextEditorDocument *doc = part->editorDocument();
    saveFileInteractionFinished(doc->fileName());
}


void Core::slotSaveFileAs()
{
    // FIXME: enable/disable this action for active DocumentionPart
    if (!manager->activePart()->inherits("EditorPart"))
        return;

    FileNameEdit *w = new FileNameEdit(i18n("Save file as:"), win->statusBar());
    EditorPart *part = static_cast<EditorPart*>(manager->activePart());
    TextEditorDocument *doc = part->editorDocument();
    QString fileName = doc->fileName();
    w->setText(fileName);
    w->show();
    w->setFocus();

    connect( w, SIGNAL(finished(const QString &)),
             this, SLOT(saveFileInteractionFinished(const QString &)) );
}


void Core::slotCloseWindow()
{
    if (manager->activePart())
        delete manager->activePart()->widget();
}


void Core::splitWindow(Orientation orient)
{
    KParts::Part *activePart = manager->activePart();
    if (!activePart)
        return;

    if (activePart->inherits("EditorPart")) {
        EditorPart *part = static_cast<EditorPart*>(manager->activePart());
        TextEditorDocument *doc = part->editorDocument();
        EditorPart *newpart = createEditorPart();
        win->splitDocumentWidget(newpart->widget(), activePart->widget(), orient);
        newpart->gotoDocument(doc, 0);
        win->raiseWidget(newpart->widget());
    } else if (activePart->inherits("DocumentationPart")) {
        DocumentationPart *part = static_cast<DocumentationPart*>(manager->activePart());
        KURL url = part->browserURL();
        DocumentationPart *newpart = createDocumentationPart();
        win->splitDocumentWidget(newpart->widget(), activePart->widget(), orient);
        newpart->gotoURL(url);
        win->raiseWidget(newpart->widget());
    }
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
    KParts::Part *activePart = manager->activePart();
    if (!activePart)
        return;

    if (activePart->inherits("EditorPart")) {
        EditorPart *part = static_cast<EditorPart*>(manager->activePart());
        TextEditorDocument *doc = part->editorDocument();
        
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
}


void Core::slotQuit()
{
    disconnect( manager, SIGNAL(activePartChanged(KParts::Part*)),
                win, SLOT(createGUI(KParts::Part*)) );

    closeProject();
    removeComponents();

    win->closeReal();
}


void Core::slotProjectOpen()
{
    QString dirName = KFileDialog::getExistingDirectory(QString::null, win, i18n("Open project"));
    if (dirName.isNull())
      return;
    if (dirName[dirName.length()-1] == '/')
        dirName.truncate(dirName.length()-1);
    
    closeProject();
    projectDir = dirName;
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
    QString fileName = sender()->name();
    gotoSourceFile(KURL(fileName), 0);
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
