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
#include <qtabdialog.h>
#include <qtextstream.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <dcopclient.h>
#include <kapp.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/partmanager.h>
#include <kparts/factory.h>
#include <kprocess.h>
#include <krun.h>
#include <kstdaction.h>
#include <kstddirs.h>
#include <ktrader.h>
#include <kconfig.h>

#include "classstore.h"
#include "kdevapi.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevproject.h"
#include "kdevlanguagesupport.h"
#include "KDevCoreIface.h"

#include "documentationpart.h"

#include "keditor/editor.h"
#include "keditor/cursor_iface.h"
#include "keditor/status_iface.h"
#include "keditor/debug_iface.h"

#include "filenameedit.h"
#include "partselectwidget.h"
#include "settingswidget.h"
#include "toplevel.h"
#include "partloader.h"
#include "core.h"


Core::Core()
    : KDevCore(), _editor(0)
{
    api = new KDevApi();
    api->core = this;
    api->classStore = new ClassStore();
    api->projectDom = 0;
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
    connect( manager, SIGNAL(partAdded(KParts::Part*)),
             this, SLOT(partCountChanged())),
    connect( manager, SIGNAL(partRemoved(KParts::Part*)),
             this, SLOT(partCountChanged()));

    initActions();
    win->createGUI(0);

    // create editor object
    (void) editor();

    initGlobalParts();

    win->show();

    emit coreInitialized();

    // load the project if needed
    KConfig* config = kapp->config();
    config->setGroup("General Options");
    QString project = config->readEntry("Last Project","");
    bool readProject = config->readBoolEntry("Read Last Project On Startup",true);
    if(project !="" && readProject){
      projectFile = project;
      openProject();
    }

    _recentProjectAction->loadEntries(config, "RecentProjects");
}


Core::~Core()
{
    delete dcopIface;
    PartLoader::cleanup();
}


KActionCollection *Core::actionCollection()
{
    return win->actionCollection();
}


void Core::initActions()
{
    KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

    KAction *action;

    action = new KAction( i18n("&Open file..."), "fileopen", CTRL+Key_O,
                          this, SLOT(slotOpenFile()),
                          actionCollection(), "file_open" );

#if 0
    // TODO: Implement with the new editor framework

    action = new KAction( i18n("Split window &vertically"), CTRL+Key_2,
                          this, SLOT(slotSplitVertically()),
                          actionCollection(), "file_splitvertically" );

    action = new KAction( i18n("Split window &horizontally"), CTRL+Key_3,
                          this, SLOT(slotSplitHorizontally()),
                          actionCollection(), "file_splithorizontally" );
#endif
    _saveAll = new KAction( i18n("Save &all"), 0,
                          this, SLOT(slotSaveAll()),
                          actionCollection(), "file_save_all");
    _saveAll->setEnabled(false);

    _revertAll = new KAction( i18n("&Revert all"), 0,
                          this, SLOT(slotRevertAll()),
                          actionCollection(), "file_revert_all");
    _revertAll->setEnabled(false);

    _closeWindow = new KAction( i18n("&Close window"), Key_F4,
                          this, SLOT(slotCloseWindow()),
                          actionCollection(), "file_closewindow" );
    _closeWindow->setEnabled(false);

    _killBuffer = new KAction( i18n("&Kill buffer"), 0,
                          this, SLOT(slotKillBuffer()),
                          actionCollection(), "file_killbuffer" );
    _killBuffer->setEnabled(false);

    action = new KAction( i18n("&Open project..."), "project_open", 0,
                          this, SLOT(slotProjectOpen()),
                          actionCollection(), "project_open" );
    action->setStatusText( i18n("Opens a project") );

    _recentProjectAction = new KRecentFilesAction( i18n("Open &recent project..."), 0,
                          this, SLOT(recentProjectSelected(const KURL &)),
                          actionCollection(), "project_open_recent");
    _recentProjectAction->setStatusText( i18n("Opens a recent project") );

    action = new KAction( i18n("C&lose project"), "fileclose",0,
                          this, SLOT(slotProjectClose()),
                          actionCollection(), "project_close" );
    action->setEnabled(false);
    action->setStatusText( i18n("Closes the current project") );

    action = new KAction( i18n("Project &Options..."), 0,
                          this, SLOT(slotProjectOptions()),
                          actionCollection(), "project_options" );
    action->setEnabled(false);

    action = KStdAction::preferences( this, SLOT(slotSettingsCustomize()),
                          actionCollection(), "settings_configure" );
    action->setStatusText( i18n("Lets you customize KDevelop") );

    action = new KAction( i18n("&Stop"), "stop", 0,
                          this, SLOT(slotStop()),
                          actionCollection(), "stop_processes");
    action->setStatusText( i18n("Stops all running subprocesses") );
    action->setEnabled(false);
}


void Core::initPart(KDevPart *part)
{
    parts.append(part);
    win->guiFactory()->addClient(part);
}


void Core::removePart(KDevPart *part)
{
    kdDebug(9000) << "Removing " << part->name() << endl;
    win->guiFactory()->removeClient(part);
    parts.remove(part);
    delete part;
}


void Core::initGlobalParts()
{
    KService *service;
    KDevPart *part;

    // Make frontend
    KTrader::OfferList makeFrontendOffers =
        KTrader::self()->query(QString::fromLatin1("KDevelop/MakeFrontend"), QString::null);
    if (makeFrontendOffers.isEmpty())
        return;
    service = *makeFrontendOffers.begin();
    part = PartLoader::loadService(service, "KDevMakeFrontend", api, this);
    if (!part)
        return;
    initPart(api->makeFrontend = static_cast<KDevMakeFrontend*>(part));

    // App frontend
    KTrader::OfferList appFrontendOffers =
        KTrader::self()->query(QString::fromLatin1("KDevelop/AppFrontend"), QString::null);
    if (appFrontendOffers.isEmpty())
        return;
    service = *appFrontendOffers.begin();
    part = PartLoader::loadService(service, "KDevAppFrontend", api, this);
    if (!part)
        return;
    initPart(api->appFrontend = static_cast<KDevAppFrontend*>(part));

    // Global parts
    KTrader::OfferList globalOffers
        = KTrader::self()->query(QString::fromLatin1("KDevelop/Part"),
                                 QString::fromLatin1("[X-KDevelop-Scope] == 'Global'"));
    KConfig *config = KGlobal::config();
    for (KTrader::OfferList::ConstIterator it = globalOffers.begin(); it != globalOffers.end(); ++it) {
    config->setGroup("Plugins");
        if (!config->readBoolEntry((*it)->name(), true)) {
            kdDebug(9000) << "Not loading " << (*it)->name() << endl;
            continue;
        }
        part = PartLoader::loadService(*it, "KDevPart", api, this);
        if (!part)
            return;
        initPart(part);
        globalParts.append(part);
    }
}


void Core::removeGlobalParts()
{
    QPtrListIterator<KDevPart> it(globalParts);
    for (; it.current(); ++it) {
        kdDebug(9000) << "Still have part " << it.current()->name() << endl;
    }

    while (!globalParts.isEmpty()) {
        removePart(globalParts.first());
        globalParts.removeFirst();
    }
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

    slotUpdateStatusBar();

    // Test if the file has been modified on the disc
    // in the meanwhile, and revert if needed.
    if (activePart && activePart->inherits("KEditor::Document"))
    {
        KEditor::Document *doc = static_cast<KEditor::Document*>(activePart);
        if (doc->modifiedOnDisc())
        {
            int res = KMessageBox::warningYesNo (win, i18n("The file %1 was modified on disk.\nRevert to the version on disk now?").arg(doc->url().url()));
            if (res == KMessageBox::Yes)
              doc->openURL(doc->url());
        }
    }
}


void Core::partCountChanged()
{
  // enable/disable actions according to the part status

  bool rw_parts = false;
  bool ro_parts = false;

  QPtrListIterator<KParts::Part> it(*partManager()->parts());
  for ( ; it.current(); ++it)
  {
      if (it.current()->inherits("KParts::ReadWritePart"))
          rw_parts = true;
      if (it.current()->inherits("KParts::ReadOnlyPart"))
          ro_parts = true;
  }

  _saveAll->setEnabled(rw_parts);
  _revertAll->setEnabled(rw_parts);
  _closeWindow->setEnabled(ro_parts);
  _killBuffer->setEnabled(ro_parts);
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


KEditor::Editor *Core::editor()
{
  // only load the editor once
  if (_editor)
    return _editor;

  // find the preferred editor
  KConfig *config = kapp->config();
  config->setGroup("Editor");
  //  QString editor = config->readEntry("EmbeddedEditor", "KWriteEditorPart"); // dosn't work at the moment
  QString editor = config->readEntry("EmbeddedEditor","EditorTestPart"); 
 

  // ask the trader about the editors, using the preferred one if available
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KDevelop/Editor"), QString("Name == '%1'").arg(editor));
 
  // try to load the editor
  KTrader::OfferList::Iterator it;
  for (it = offers.begin(); it != offers.end(); ++it)
  {
    KLibFactory *factory = KLibLoader::self()->factory((*it)->library().latin1());
    if (!factory)
      continue;

    // Note: The create function might return 0, in that
    // case, we continue with our list
    _editor = static_cast<KEditor::Editor*>(factory->create(win, "editor"));
    if (_editor)
      break;
  }

  // Note: We should probably abort the application if no editor
  // is found at all!
  if (!_editor){
    KMessageBox::sorry(win, i18n("Can't find a Editor plugin :-(."));
    exit(0);
  }

  // merge the GUI with ours
  win->factory()->addClient(_editor);

  // make the editor aware of part activations
  connect(partManager(), SIGNAL(activePartChanged(KParts::Part*)), _editor, SLOT(activePartChanged(KParts::Part*)));

  return _editor;
}


void Core::editorContextMenu(QPopupMenu *popup, const QString &linestr, int col)
{
    EditorContext context(linestr, col);
    emit contextMenu(popup, &context);
}



void Core::updateBufferMenu()
{
    // TODO: Fix the documentation parts so they work properly

    QPtrList<KAction> bufferActions;

    win->unplugActionList("buffer_list");

    QPtrListIterator<KParts::Part> it(*partManager()->parts());
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
}


void Core::closeProject()
{
    if  (api->project) {
        emit projectClosed();
        api->project->closeProject();

        while (!localParts.isEmpty()) {
            removePart(localParts.first());
            localParts.removeFirst();
        }

        if (api->languageSupport) {
            removePart(api->languageSupport);
            api->languageSupport = 0;
        }

        if (api->projectDom) {
            QFile fout(projectFile);
            if (fout.open(IO_WriteOnly)) {
                QTextStream stream(&fout);
                api->projectDom->save(stream, 2);
            } else {
                KMessageBox::sorry(win, i18n("Could not write the project file."));
            }
            fout.close();
            delete api->projectDom;
            api->projectDom = 0;
        }
        removePart(api->project);
        api->project = 0;
    }

	api->classStore->wipeout();

    projectFile = QString::null;
    win->setCaption(QString::fromLatin1(""));
    actionCollection()->action("project_close")->setEnabled(false);
    actionCollection()->action("project_options")->setEnabled(false);
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
    api->projectDom = doc;
    fin.close();

    QDomElement docEl = api->projectDom->documentElement();
    QDomElement generalEl = docEl.namedItem("general").toElement();
    QDomElement projectEl = generalEl.namedItem("projectmanagement").toElement();

    QString projectPlugin = projectEl.firstChild().toText().data();
    kdDebug(9000) << "Project plugin: " << projectPlugin << endl;

    QDomElement primarylanguageEl = generalEl.namedItem("primarylanguage").toElement();
    QString language = primarylanguageEl.firstChild().toText().data();
    kdDebug(9000) << "Primary language: " << language << endl;

    // check for ignore parts
    QStringList ignoreparts;
    QDomElement ignorepartsEl = generalEl.namedItem("ignoreparts").toElement();
    QDomElement partEl = ignorepartsEl.firstChild().toElement();
    while (!partEl.isNull()) {
      if (partEl.tagName() == "part")
	ignoreparts << partEl.firstChild().toText().data();
      partEl = partEl.nextSibling().toElement();
    }
    // check for load parts
    QStringList loadparts;
    QDomElement loadpartsEl = generalEl.namedItem("loadparts").toElement();
    QDomElement loadpartEl = loadpartsEl.firstChild().toElement();
    while (!loadpartEl.isNull()) {
        if (loadpartEl.tagName() == "part")
            loadparts << loadpartEl.firstChild().toText().data();
        loadpartEl = loadpartEl.nextSibling().toElement();
    }
    // check for keywords
    QStringList keywords;
    QDomElement keywordsEl = generalEl.namedItem("keywords").toElement();
    QDomElement keywordEl = keywordsEl.firstChild().toElement();
    while (!keywordEl.isNull()) {
      if (keywordEl.tagName() == "keyword")
	keywords << keywordEl.firstChild().toText().data();
      keywordEl = keywordEl.nextSibling().toElement();
    }

    // Load project part
    KService::Ptr projectService = KService::serviceByName(projectPlugin);
    if (projectService) {
        KDevPart *part = PartLoader::loadService(projectService, "KDevProject", api, this);
        if (!part)
            return;
        initPart(api->project = static_cast<KDevProject*>(part));
    } else {
        KMessageBox::sorry(win, i18n("No project management plugin %1 found.").arg(projectPlugin));
        return;
    }

    // Load language support part
    KTrader::OfferList languageSupportOffers
        = KTrader::self()->query(QString::fromLatin1("KDevelop/LanguageSupport"),
                                 QString::fromLatin1("[X-KDevelop-Language] == '%1'").arg(language));
    if (!languageSupportOffers.isEmpty()) {
        KService *languageSupportService = *languageSupportOffers.begin();
        KDevPart *part = PartLoader::loadService(languageSupportService, "KDevLanguageSupport", api, this);
        if (!part)
            return;
        initPart(api->languageSupport = static_cast<KDevLanguageSupport*>(part));
    } else
        KMessageBox::sorry(win, i18n("No language plugin for %1 found.").arg(language));



    // Load local parts
    KTrader::OfferList localOffers
        = KTrader::self()->query(QString::fromLatin1("KDevelop/Part"),
                                 QString::fromLatin1("[X-KDevelop-Scope] == 'Project'"));
    for (KTrader::OfferList::ConstIterator it = localOffers.begin(); it != localOffers.end(); ++it) {

      if (ignoreparts.contains((*it)->name())){
	continue; // do nothing
      }

      if (loadparts.contains((*it)->name())){ // load the part if in loadparts
	KDevPart *part = PartLoader::loadService(*it, "KDevPart", api, this);
        if (!part)
            return;
	initPart(part);
	localParts.append(part);
      }

      else { // check if the "new" part should be loaded or ignored
	QVariant var = (*it)->property("X-KDevelop-ProgrammingLanguages");
	QStringList langlist = var.asStringList();
	if(langlist.contains(language) || langlist.isEmpty()){ // empty means it support all languages
	  // the language is ok, now check if the keywords match
	  bool keywordsMatch = true;
	  QStringList serviceKeywords = (*it)->keywords();
	  QStringList::Iterator is = serviceKeywords.begin();
	  while ( is != serviceKeywords.end() ){
	    if(keywords.contains(*is) ==0 && keywordsMatch){ // no match
	      keywordsMatch = false;
	      kdDebug(9000) << "ignoreParts because Keyword doesn't match: " << (*it)->name() << endl;
	      ignoreparts << (*it)->name();
	    }
	    is++;
	  }
	  // the language and all keywords match or no keywords available
	  if(keywordsMatch){
	    loadparts << (*it)->name();
	    KDevPart *part = PartLoader::loadService(*it, "KDevPart", api, this);
            if (!part)
                return;
	    initPart(part);
	    localParts.append(part);
	  }
	}
	else {
	  // the language doesn't match
	  ignoreparts << (*it)->name();
	}
      }
    }

    // store the ignore parts to the projectfile
    if (ignorepartsEl.isNull()) {
      ignorepartsEl = api->projectDom->createElement("ignoreparts");
      generalEl.appendChild(ignorepartsEl);
    }

    // Clear old entries
    while (!ignorepartsEl.firstChild().isNull()){
      ignorepartsEl.removeChild(ignorepartsEl.firstChild());
    }
    QStringList::Iterator ignoreIterator = ignoreparts.begin();
    while ( ignoreIterator != ignoreparts.end()){
      QDomElement partEl = api->projectDom->createElement("part");
      partEl.appendChild(api->projectDom->createTextNode(*ignoreIterator));
      ignorepartsEl.appendChild(partEl);
      ignoreIterator++;
    }

    // store the loaded parts
    if (loadpartsEl.isNull()) {
      loadpartsEl = api->projectDom->createElement("loadparts");
      generalEl.appendChild(loadpartsEl);
    }

    // Clear old entries
    while (!loadpartsEl.firstChild().isNull()){
      loadpartsEl.removeChild(loadpartsEl.firstChild());
    }
    QStringList::Iterator loadIterator = loadparts.begin();
    while ( loadIterator != loadparts.end()){
      QDomElement partEl = api->projectDom->createElement("part");
      partEl.appendChild(api->projectDom->createTextNode(*loadIterator));
      loadpartsEl.appendChild(partEl);
      loadIterator++;
    }

    QFileInfo fi(projectFile);
    QString projectDir = fi.dirPath();
    kdDebug(9000) << "projectDir: " << projectDir << endl;

    if (api->project)
        api->project->openProject(projectDir);
    emit projectOpened();
    _recentProjectAction->addURL(projectFile);

    win->setCaption(projectDir);
    actionCollection()->action("project_close")->setEnabled(true);
    actionCollection()->action("project_options")->setEnabled(true);
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
    QString mimeType = KMimeType::findByURL(url, 0, true, true)->name();
    if (!mimeType.startsWith("text/")) {
      	KParts::Factory *factory = 0;

        // try to find a ReadWrite part that handles this type
       	KTrader::OfferList offers = KTrader::self()->query(mimeType, "KParts/ReadWritePart", QString::null, QString::null);
        if (offers.count() > 0) {
            // use the first offer
            KService::Ptr ptr = offers.first();
            factory = static_cast<KParts::Factory*>(KLibLoader::self()->factory(ptr->library().latin1()));
        }

        // if no ReadWritePart was available, try a ReadOnlyPart
        if (!factory) {
            offers = KTrader::self()->query(mimeType, "KParts/ReadOnlyPart", QString::null, QString::null);
            if (offers.count() > 0) {
                // use the first offer
                KService::Ptr ptr = offers.first();
                factory = static_cast<KParts::Factory*>(KLibLoader::self()->factory(ptr->library().latin1()));
	    }
    	}

        if (factory) {
            // create a part
            KParts::ReadOnlyPart *part = static_cast<KParts::ReadOnlyPart*>(factory->createPart(win));

            // embed widget
            win->embedDocumentWidget(part->widget(), activePart ? activePart->widget() : 0);

            // inform the part manager
            partManager()->addPart(part);

            // open the url
            part->openURL(url);

            updateBufferMenu();

            return;
        }

        // try to start a new handling process
        new KRun(url);
    } else {
        gotoSourceFile(url, 0, Replace);
    }
}


void Core::gotoDocumentationFile(const KURL& url, Embedding embed)
{
    kdDebug(9000) << "Goto documentation: " << url.url() << endl;

    // See if we have this url already loaded.
    KURL::List::Iterator it;
    for (it = viewedURLs.begin(); it != viewedURLs.end(); ++it)
        if ((*it) == url)
            break;

    if (it == viewedURLs.end()) {
        kdDebug(9000) << "url not yet used" << endl;
        viewedURLs.append(url);
        updateBufferMenu();
    }

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


KEditor::Document *Core::createDocument(const KURL &url)
{
  KEditor::Document *doc = editor()->createDocument(win, url);
  if (!doc)
      return 0;

  connect(doc, SIGNAL(loaded(KEditor::Document*)), this, SLOT(slotDocumentLoaded(KEditor::Document*)));
  connect(doc, SIGNAL(saved(KEditor::Document*)), this, SLOT(slotDocumentSaved(KEditor::Document*)));

  KEditor::StatusDocumentIface *status = KEditor::StatusDocumentIface::interface(doc);
  if (status)
  {
    connect(status, SIGNAL(message(KEditor::Document*,const QString &)), this, SLOT(message(KEditor::Document*,const QString &))),
    connect(status, SIGNAL(statusChanged(KEditor::Document*)), this, SLOT(slotUpdateStatusBar()));
  }

  KEditor::CursorDocumentIface *cursor = KEditor::CursorDocumentIface::interface(doc);
 if (cursor)
   connect(cursor, SIGNAL(cursorPositionChanged(KEditor::Document*,int,int)), this, SLOT(slotUpdateStatusBar()));

  KEditor::DebugDocumentIface *debug = KEditor::DebugDocumentIface::interface(doc);
  if (debug)
  {
    connect(debug, SIGNAL(breakPointToggled(KEditor::Document*,int)), this, SLOT(slotBreakPointToggled(KEditor::Document*,int)));
    connect(debug, SIGNAL(breakPointEnabledToggled(KEditor::Document*,int)), this, SLOT(slotBreakPointEnabled(KEditor::Document*,int)));
  }

  partManager()->addPart(doc);

  return doc;
}


void Core::gotoSourceFile(const KURL& url, int lineNum, Embedding embed)
{
    if (url.isEmpty())
      return;

    kdDebug(9000) << "Goto source file: " << url.path() << " line: " << lineNum << endl;

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
    KEditor::CursorDocumentIface *iface = KEditor::CursorDocumentIface::interface(doc);
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
}


void Core::gotoExecutionPoint(const QString &fileName, int lineNum)
{
    KURL url(fileName);

    // TODO: This needs fixing: Disable all others,
    // load the file if not there yet.

    gotoSourceFile(url, lineNum);

    KEditor::Document *doc = editor()->document(url);
    if (!doc)
      return;

    KEditor::DebugDocumentIface *debug = KEditor::DebugDocumentIface::interface(doc);
    if (!debug)
      return;

    debug->markExecutionPoint(lineNum);
}


void Core::revertAllFiles()
{
    QPtrListIterator<KParts::Part> it(*partManager()->parts());
    for ( ; it.current(); ++it)
    {
        if (it.current()->inherits("KParts::ReadWritePart"))
        {
            KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(it.current());

            rw_part->openURL(rw_part->url());
        }
    }
}


void Core::saveAllFiles()
{
    QPtrListIterator<KParts::Part> it(*partManager()->parts());
    for ( ; it.current(); ++it)
        if (it.current()->inherits("KParts::ReadWritePart"))
        {
            KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(it.current());

            rw_part->save();

            message(i18n("Saved %1").arg(rw_part->url().url()));
        }
}


void Core::setBreakpoint(const QString &fileName, int lineNum,
                         int id, bool enabled, bool pending)
{
    KURL url(fileName);

    KEditor::Document *doc = editor()->document(url);
    if (!doc)
      return;

    KEditor::DebugDocumentIface *debug = KEditor::DebugDocumentIface::interface(doc);
    if (!debug)
      return;

    if (id >= 0)
      debug->setBreakPoint(lineNum, enabled, pending);
    else
      debug->unsetBreakPoint(lineNum);
}


void Core::running(KDevPart *part, bool runs)
{
    if (runs)
        runningParts.append(part);
    else
        runningParts.remove(part);

    actionCollection()->action("stop_processes")->setEnabled(!runningParts.isEmpty());
}


void Core::fillContextMenu(QPopupMenu *popup, const Context *context)
{
    emit contextMenu(popup, context);
}


void Core::message(KEditor::Document *, const QString &str)
{
    win->statusBar()->message(str);
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
    gotoFile(KURL(fileName));
}


void Core::slotOpenFile()
{
#if 1
    QString fileName = KFileDialog::getOpenFileName();
    if (!fileName.isEmpty())
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


void Core::slotCloseWindow()
{
    if (!activePart || !activePart->inherits("KParts::ReadWritePart"))
        return;

    KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(activePart);

    if (rw_part->isModified())
    {
        int res = KMessageBox::warningYesNo(win,
                     i18n("The file %1 is modified.\n"
                          "Close this window anyway?").arg(rw_part->url().url()));
        if (res == KMessageBox::No)
            return;
    }

    delete activePart;
}


void Core::splitWindow(Orientation orient)
{
    if (!activePart)
        return;
#if 0

    // TODO: Implement splitting

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
#if 0
    if (!activePart)
        return;

    if (activePart->inherits("EditorPart")) {
        EditorPart *part = static_cast<EditorPart*>(activePart);
        TextEditorDocument *doc = part->editorDocument();
        if (doc->isModified() && !KMessageBox::warningYesNo
            (win, i18n("The file %1 is modified.\n"
                       "Close this window anyway?").arg(doc->url().url())))
            return;

        QPtrList<EditorPart> deletedParts;

        QPtrListIterator<EditorPart> it1(editorParts);
        for (; it1.current(); ++it1) {
            if (it1.current()->editorDocument() == doc)
                deletedParts.append(it1.current());
        }

        // Kill all parts that are still looking at this document
        // - One may want to change this to Emacs' behaviour
        QPtrListIterator<EditorPart> it2(deletedParts);
        for (; it2.current(); ++it2)
            delete it2.current()->widget();

        editedDocs.remove(doc);
        updateBufferMenu();
        delete doc;
    } else if (activePart->inherits("DocumentationPart")) {
        DocumentationPart *part = static_cast<DocumentationPart*>(activePart);
        KURL url = part->browserURL();

        QPtrList<DocumentationPart> deletedParts;

        QPtrListIterator<DocumentationPart> it1(docParts);
        for (; it1.current(); ++it1) {
            if (it1.current()->browserURL() == url)
                deletedParts.append(it1.current());
        }

        // Kill all parts that are still looking at this url
        // - One may want to change this to Emacs' behaviour
        QPtrListIterator<DocumentationPart> it2(deletedParts);
        for (; it2.current(); ++it2)
            delete it2.current()->widget();

        viewedURLs.remove(url);
        updateBufferMenu();
    }
#endif
}


void Core::slotQuit()
{
    QPtrListIterator<KParts::Part> it(*partManager()->parts());
    for ( ; it.current(); ++it)
        {
            if (!it.current()->inherits("KParts::ReadWritePart"))
                continue;

            KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(it.current());

            if (rw_part->isModified())
                {
                    int res = KMessageBox::warningYesNoCancel(win,
                                i18n("The file %1 is modified.\n"
                                     "Save this file now?").arg(rw_part->url().url()));
                    if (res == KMessageBox::Yes)
                        rw_part->save();
                    if (res == KMessageBox::Cancel)
                        return;
                }
        }

    disconnect( manager, SIGNAL(activePartChanged(KParts::Part*)),
                this, SLOT(activePartChanged(KParts::Part*)) );

    // save the the project to open it automaticly on startup if needed
    KConfig* config = kapp->config();
    config->setGroup("General Options");
    config->writeEntry("Last Project",projectFile);
    closeProject();
    removeGlobalParts();

    // save the recent projects
    _recentProjectAction->saveEntries(config, "RecentProjects");

    win->closeReal();
}


void Core::slotSaveAll()
{
    saveAllFiles();
}


void Core::slotRevertAll()
{
    revertAllFiles();
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


void Core::slotBufferSelected()
{
    // Note: this code is intented to work with more than
    // just editor parts, e.g. with documentation parts. There
    // are two problems right now: a) the documentation part's
    // implementation is broken, and b) we need a better mechanism
    // than the URL to identify the parts.

    QPtrListIterator<KParts::Part> it(*partManager()->parts());
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
}


void Core::slotDocumentationBufferSelected(const KURL &url)
{
    gotoDocumentationFile(url);
}


void Core::slotProjectOptions()
{
    KDialogBase dlg(KDialogBase::TreeList, i18n("Project Options"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, win,
                    "project options dialog");

    QVBox *vbox = dlg.addVBoxPage(i18n("Plugins"));
    PartSelectWidget *w = new PartSelectWidget(*api->projectDom, vbox, "part selection widget");
    connect( &dlg, SIGNAL(okClicked()), w, SLOT(accept()) );

    emit projectConfigWidget(&dlg);
    dlg.exec();
}


void Core::slotSettingsCustomize()
{
    KDialogBase dlg(KDialogBase::TreeList, i18n("Customize KDevelop"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, win,
                    "customization dialog");

    QVBox *vbox = dlg.addVBoxPage(i18n("General"));
    SettingsWidget *gsw = new SettingsWidget(vbox, "general settings widget");
    KConfig* config = kapp->config();
    config->setGroup("General Options");
    gsw->lastProjectCheckbox->setChecked(config->readBoolEntry("Read Last Project On Startup",true));


    vbox = dlg.addVBoxPage(i18n("Plugins"));
    PartSelectWidget *w = new PartSelectWidget(vbox, "part selection widget");
    connect( &dlg, SIGNAL(okClicked()), w, SLOT(accept()) );

    emit configWidget(&dlg);
    dlg.exec();

    config->setGroup("General Options");
    config->writeEntry("Read Last Project On Startup",gsw->lastProjectCheckbox->isChecked());
}


void Core::slotStop()
{
    // Hmm, not much to do ;-)
    emit stopButtonClicked();
}


void Core::slotUpdateStatusBar()
{
  if (!activePart || !activePart->inherits("KEditor::Document"))
  {
    win->statusBar()->setEditorStatusVisible(false);
    return;
  }

  KEditor::Document *doc = static_cast<KEditor::Document*>(activePart);

  KEditor::StatusDocumentIface *status = KEditor::StatusDocumentIface::interface(doc);
  if (status)
  {
    win->statusBar()->setStatus(status->status());
    win->statusBar()->setModified(status->modified());
  }

  KEditor::CursorDocumentIface *cursor = KEditor::CursorDocumentIface::interface(doc);
  if (cursor)
  {
     int line, col;
     cursor->getCursorPosition(line, col);
     win->statusBar()->setCursorPosition(line, col);
  }

  win->statusBar()->setEditorStatusVisible(true);
}


void Core::slotBreakPointToggled(KEditor::Document *doc, int line)
{
  QString fname = doc->url().path();
  emit toggledBreakpoint(fname, line);
}


void Core::slotBreakPointEnabled(KEditor::Document *doc, int line)
{
  QString fname = doc->url().path();
  emit toggledBreakpoint(fname, line);
}


void Core::slotDocumentLoaded(KEditor::Document *doc)
{
  kdDebug() << "EMIT: loaded " << doc->url().path() << endl;
  emit loadedFile(doc->url().path());
}


void Core::slotDocumentSaved(KEditor::Document *doc)
{
  kdDebug() << "EMIT: saved " << doc->url().path() << endl;
  emit savedFile(doc->url().path());
}


QStatusBar *Core::statusBar() const
{
  return win->statusBar();
}


void Core::recentProjectSelected(const KURL &url)
{
  projectFile = url.path();
  openProject();
}


//#include "core.moc"
