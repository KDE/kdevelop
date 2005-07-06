/***************************************************************************
 *   Copyright (C) 2005 by Jens Herden                                     *
 *   jens@kdewebdev.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "projectviewprojectconfig.h"
#include "projectviewconfig.h"
#include "projectviewpart.h"
#include "filelist_widget.h"
#include "toolbarguibuilder.h"

#include <qtimer.h>
#include <qpopupmenu.h>
#include <qwhatsthis.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qdir.h>
#include <qdom.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qpoint.h>
#include <qlayout.h>

#include <kmainwindow.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kaction.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ktexteditor/viewcursorinterface.h>
#include <kparts/part.h>
#include <kxmlguibuilder.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>

#include <kdevplugininfo.h>
#include <kdevgenericfactory.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <configwidgetproxy.h>



typedef KDevGenericFactory<ProjectviewPart> projectviewFactory;
KDevPluginInfo data("kdevfilelist");
K_EXPORT_COMPONENT_FACTORY(libkdevfilelist, projectviewFactory(data));

#define GLOBALDOC_OPTIONS 1
#define PROJECTDOC_OPTIONS 2

ProjectviewPart::ProjectviewPart(QObject *parent, const char *name, const QStringList &/*args*/)
    : KDevPlugin(&data, parent, name ? name : "FileListPart")
{
    setInstance(projectviewFactory::instance());
    setXMLFile("kdevfilelist.rc");
    setupActions();
    
    m_configProxy = new ConfigWidgetProxy(core());
    m_configProxy->createGlobalConfigPage(i18n("File List"), GLOBALDOC_OPTIONS, info()->icon());
    m_configProxy->createProjectConfigPage(i18n("File List"), PROJECTDOC_OPTIONS, info()->icon());

    connect(m_configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int)),
            this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int)));
    
    connect(core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
            this, SLOT(contextMenu(QPopupMenu *, const Context *)));
            
    connect(core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()));
    connect(core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()));
  
    KConfig * config = kapp->config();
    config->setGroup("File List Plugin");
    if (config->readBoolEntry("ToolbarInToolview", true))
    {
      m_toolbarWidget = new QVBox(0, "toolbarContainer");
      m_toolbarWidget->setHidden(true);
      m_guibuilder = new ToolbarGUIBuilder(m_toolbarWidget, mainWindow()->main()); 
      setClientBuilder(m_guibuilder);
    }
    QTimer::singleShot(0, this, SLOT(init()));
}

ProjectviewPart::~ProjectviewPart()
{
  delete m_configProxy;
  delete m_guibuilder;
  delete m_widget;  // deletes the children as well
}

void ProjectviewPart::restorePartialProjectSession(const QDomElement * el)
{
    if (!el)
    {
        return;
    }
    // get the base of the project
    QString urlStr = project()->projectDirectory();
    if (KURL::isRelativeURL(urlStr))
    {
        m_projectBase.setProtocol("file");
        m_projectBase.setPath(urlStr);
    }else
    {
        m_projectBase = KURL::fromPathOrURL(urlStr);
    }
    m_projectBase.adjustPath(+1);  // just in case
    
    // read all the views
    m_projectViews.clear();
    QDomNodeList domList = el->elementsByTagName("projectview");
    
    uint len = domList.length();
    for (uint i = 0; i < len; ++i)
    {
        const QDomElement viewEl = domList.item(i).toElement();
        if (viewEl.isNull())
        {
            continue;
        }
        FileInfoList urlList;
        QDomNodeList fileList = viewEl.elementsByTagName("file");

        uint len2 = fileList.length();
        for (uint i2 = 0; i2 < len2; ++i2)
        {
            const QDomElement fileEl = fileList.item(i2).toElement();
            if (!fileEl.isNull())
            {
                bool ok;
                int line;
                QString attr = fileEl.attribute("line");
                if (! attr.isNull()) 
                {
                    line = attr.toInt(&ok);
                    if (!ok)
                        line = -1;
                }
                int col;
                attr = fileEl.attribute("col");
                if (! attr.isNull())
                {
                    col = attr.toInt(&ok);
                    if (!ok)
                        col = -1;
                }
                QString urlStr = fileEl.attribute("url");
                if (KURL::isRelativeURL(urlStr))
                {
                    KURL url = m_projectBase;
                    url.addPath(urlStr);
                    urlList.append(FileInfo(url, line, col));
                } else
                    urlList.append(FileInfo(KURL::fromPathOrURL(urlStr), line, col));
            }
        }
        m_projectViews.insert(viewEl.attribute("name"), urlList);
    }
    // read default view
    domList = el->elementsByTagName("defaultview");
    if (domList.length() > 0)
    {
        m_defaultProjectView = domList.item(0).toElement().attribute("name");
        if (!m_defaultProjectView.isEmpty()) 
        {
            slotOpenProjectView(m_defaultProjectView);
        } 
    } else 
    {
        m_defaultProjectView = "";
    }
    adjustViewActions();
}

void ProjectviewPart::savePartialProjectSession(QDomElement * el)
{
    if (!el || m_projectViews.empty())
    {
        return;
    }
    QDomDocument domDoc = el->ownerDocument();
    if (domDoc.isNull())
    {
        return;
    }
    // write all views
    ViewMap::ConstIterator it;
    for (it = m_projectViews.constBegin(); it != m_projectViews.constEnd(); ++it)
    {
        // we create the view even if there is no file inside, might be wanted by the user
        QDomElement viewEl = domDoc.createElement("projectview");
        viewEl.setAttribute("name", it.key());
        el->appendChild(viewEl);
        for (FileInfoList::ConstIterator it2 = it.data().constBegin(); it2 != it.data().constEnd(); ++it2)
        {
            QDomElement urlEl = domDoc.createElement("file");
            
            if (m_projectBase.isParentOf((*it2).url))
                urlEl.setAttribute("url", KURL::relativeURL(m_projectBase, (*it2).url));
            else
                urlEl.setAttribute("url", (*it2).url.url());
            urlEl.setAttribute("line", (*it2).line);
            urlEl.setAttribute("col", (*it2).col);
            viewEl.appendChild(urlEl);
        }
    }
    // write the default view
    QDomElement defaultEl = domDoc.createElement("defaultview");
    defaultEl.setAttribute("name", m_defaultProjectView);
    el->appendChild(defaultEl);
}


void ProjectviewPart::init()
{
  // delayed initialization stuff goes here
  
  // create the toolview
  m_widget = new QWidget(0, "filelist widget");
  m_widget->setIcon( SmallIcon(info()->icon()) );
  
  QBoxLayout * l = new QVBoxLayout(m_widget);
  
  // create the toolbar if needed
  if (m_guibuilder)
  {
    m_toolbarWidget->reparent(m_widget, QPoint(0, 0), true);
    l->addWidget(m_toolbarWidget);
    QWhatsThis::add(m_toolbarWidget, i18n("<b>Project View Toolbar</b><p>This allows to create and work with project views. A project view is a set of open documents.</p>"));
  }
  
  // create the listview 
  QWidget * fileListWidget = new FileListWidget(this, m_widget);
  fileListWidget->setCaption(i18n("File List"));
  QWhatsThis::add(fileListWidget, i18n("<b>File List</b><p>This is the list of opened files.</p>"));
  l->addWidget(fileListWidget);
  
  mainWindow()->embedSelectView(m_widget, i18n("File List"), i18n("Open files"));
  
  if (!project())
    readConfig();
}

void ProjectviewPart::setupActions()
{
    m_openPrjViewAction = new KSelectAction(i18n("Open Project View..."), 0, actionCollection(), "projectviews_open");
    
    connect(m_openPrjViewAction, SIGNAL(activated(const QString &)), this, SLOT(slotOpenProjectView(const QString &)));
    
    m_openPrjViewAction->setToolTip(i18n("Open Project View"));
    
    m_savePrjViewAction = new KAction(i18n("Save Project View"), "filesave", 0, this, SLOT(slotSaveProjectView()), actionCollection(), "projectviews_save");
    
    m_newPrjViewAction = new KAction(i18n("New Project View..."), "filenew", 0, this, SLOT(slotSaveAsProjectView()), actionCollection(), "projectviews_new");
    
    m_deletePrjViewAction = new KSelectAction(i18n("Delete Project View"), "editdelete", 0, actionCollection(), "projectviews_delete");
    
    connect(m_deletePrjViewAction, SIGNAL(activated(const QString &)), this, SLOT(slotDeleteProjectView(const QString &)));
    
    m_deletePrjViewAction->setToolTip(i18n("Delete Project View"));
    
    m_deleteCurrentPrjViewAction = new KAction(i18n("Delete Project View"), "editdelete", 0, this, SLOT(slotDeleteProjectViewCurent()), actionCollection(), "projectviews_deletecurrent");
    
    m_deleteCurrentPrjViewAction->setToolTip(i18n("Delete Project View"));
    
    
    // adjust the actions
    adjustViewActions();
}

void ProjectviewPart::insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo)
{
// create configuraton dialogs here
    switch (pageNo)
    {
        case PROJECTDOC_OPTIONS:
        {
            ProjectviewProjectConfig *w = new ProjectviewProjectConfig(this, page, "project config");
            connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
            break;
        }
        case GLOBALDOC_OPTIONS:
        {
            ProjectviewConfig *w = new ProjectviewConfig(page, "global config");
            connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
            break;
        }
    }
}

void ProjectviewPart::contextMenu(QPopupMenu */*popup*/, const Context */*context*/)
{
// put actions into the context menu here
//     if (context->hasType(Context::EditorContext))
//     {
//         // editor context menu
//         const EditorContext *econtext = static_cast<const EditorContext*>(context);
// 
//         // use context and plug actions here
//         // action->plug(popup);
// 
//         // or create menu items on the fly
//         // int id = -1;
//         // id = popup->insertItem(i18n("Do Something Here"),
//         //     this, SLOT(doSomething()));
//         // popup->setWhatsThis(id, i18n("<b>Do something here</b><p>Describe here what does this action do."
//     }
//     else if (context->hasType(Context::FileContext))
//     {
//         // file context menu
//         const FileContext *fcontext = static_cast<const FileContext*>(context);
// 
//         //use context and plug actions here
//     }
//     else if (context->hasType(Context::ProjectModelItemContext))
//     {
//         // project tree context menu
//         const ProjectModelItemContext *pcontext = static_cast<const ProjectModelItemContext*>(context);
// 
//         // use context and plug actions here
//     }
//     else if (context->hasType(Context::CodeModelItemContext))
//     {
//         // class tree context menu
//         const CodeModelItemContext *mcontext = static_cast<const CodeModelItemContext*>(context);
// 
//         // use context and plug actions here
//     }
//     else if (context->hasType(Context::DocumentationContext))
//     {
//         // documentation viewer context menu
//         const DocumentationContext *dcontext = static_cast<const DocumentationContext*>(context);
// 
//         // use context and plug actions here
//     }
}

void ProjectviewPart::projectOpened()
{
}

void ProjectviewPart::projectClosed()
{
    m_projectBase = KURL();
    
    m_projectViews.clear();
    readConfig();
    m_currentProjectView = "";
    m_defaultProjectView = "";
    adjustViewActions();
}

void ProjectviewPart::slotOpenProjectView(const QString &view)
{
    KConfig * config = kapp->config();
    config->setGroup("File List Plugin");
    bool onlyProject = config->readBoolEntry("OnlyProjectFiles", false);
    
    m_currentProjectView = view;
    
    if (m_projectViews.contains(view) > 0)
    {
        FileInfoList viewUrls = m_projectViews[view];
        
        // we close everything that is not part of the project view
        KURL::List urlsToClose = partController()->openURLs();
        for (KURL::List::Iterator it = urlsToClose.begin(); it != urlsToClose.end(); ++it)
        {
            // it is in the list of wanted files and do we want it at all
            if ((viewUrls.contains(*it) > 0) && (!onlyProject || !project() ||  project()->isProjectFile((*it).path()) ))
            {
                viewUrls.remove(*it); // don't open if it is open already
                it = urlsToClose.remove(it);
                --it;  //  do not skip one
            }
        }
        if (!urlsToClose.empty())
        {
            partController()->closeFiles(urlsToClose);
        }
        // we open what still needs to get opened
        FileInfoList::const_iterator viewIt;
        for (viewIt = viewUrls.begin(); viewIt != viewUrls.end(); ++viewIt)
        {
            if (!onlyProject || !project() || project()->isProjectFile((*viewIt).url.path()))
            {
                partController()->editDocument((*viewIt).url, (*viewIt).line, (*viewIt).col); 
            }
        }
    }
    // adjust the actions
    adjustViewActions();
}


void ProjectviewPart::slotSaveProjectView()
{
    slotSaveAsProjectView(m_currentProjectView.isEmpty());
}


void ProjectviewPart::adjustViewActions()
{
    QStringList viewList = getViewList();
    
    m_openPrjViewAction->clear();
    m_openPrjViewAction->setItems(viewList);
    int i = viewList.findIndex(m_currentProjectView);
    if (i > -1)
    {
        m_openPrjViewAction->setCurrentItem(i);
    }
    m_deletePrjViewAction->clear();
    m_deletePrjViewAction->setItems(viewList);
    
    bool haveView = !(m_currentProjectView.isEmpty() || m_currentProjectView.isNull());
    m_savePrjViewAction->setEnabled(haveView);
    m_deleteCurrentPrjViewAction->setEnabled(haveView);
}


void ProjectviewPart::slotDeleteProjectViewCurent()
{
  slotDeleteProjectView(m_currentProjectView);
}

void ProjectviewPart::slotDeleteProjectView(const QString& view)
{
    m_projectViews.remove(view);
    
    if (m_currentProjectView == view)
        m_currentProjectView = "";
    
    if (m_defaultProjectView == view)
        m_defaultProjectView = "";
    
    if (! project())
      writeConfig();

    adjustViewActions();
}


void ProjectviewPart::slotSaveAsProjectView(bool askForName)
{
    if (askForName)
    {
        bool ok;
        QString newProjectView = KInputDialog::getText(i18n("Save Project View As"), i18n("Enter the name of the view:"), "", &ok, mainWindow()->main());
        if (!ok)
        {
            return;
        }
        newProjectView = newProjectView.remove("="); // we use this string in config files and = would confuse it
        if (m_projectViews.contains(newProjectView) > 0 &&
            KMessageBox::warningYesNo(mainWindow()->main(), i18n("<qt>A project view named <b>%1</b> already exists.<br>Do you want to overwrite it?</qt>").arg(newProjectView)) != KMessageBox::Yes)
        {
            return;
        }
        m_currentProjectView = newProjectView;
    }
    
    FileInfoList viewUrls;
    KURL::List openURLs = partController()->openURLs();
    
    for (KURL::List::Iterator it = openURLs.begin(); it != openURLs.end(); ++it)
    {
        // test if we have an editor
        // FIXME this can fail if there are two parts with the same URL
        KParts::ReadOnlyPart *ro_part = partController()->partForURL(*it);
        KTextEditor::ViewCursorInterface* cursorIf = dynamic_cast<KTextEditor::ViewCursorInterface*>(ro_part->widget());
        if (cursorIf)
        {
            unsigned int line, col;
            cursorIf->cursorPositionReal(&line, &col);
            viewUrls.append(FileInfo(*it, line, col));
        }
    }
    // add or overwrite the values
    m_projectViews.insert(m_currentProjectView, viewUrls, true);
    if (! project())
      writeConfig();

    adjustViewActions();
}


void ProjectviewPart::writeConfig()
{
  KConfig * config = projectviewFactory::instance()->config();
  config->deleteGroup("ProjectViews", true);
  config->setGroup("ProjectViews");
    
    // write all views
  ViewMap::ConstIterator it;
  for (it = m_projectViews.constBegin(); it != m_projectViews.constEnd(); ++it)
  {
    // we create the view even if there is no file inside, might be wanted by the user
    QStringList urls;
    for (FileInfoList::ConstIterator it2 = it.data().constBegin(); it2 != it.data().constEnd(); ++it2)
    {
      urls.append((*it2).url.url());
    }
    config->writeEntry(it.key(), urls);
  }
}


void ProjectviewPart::readConfig()
{
  KConfig * config = projectviewFactory::instance()->config();
  QMap<QString, QString> entries = config->entryMap("ProjectViews");
  
  m_projectViews.clear();
  QMap<QString, QString>::ConstIterator it;
  for (it = entries.constBegin(); it != entries.constEnd(); ++it)
  {
    FileInfoList urlList;
    QStringList urls = QStringList::split(",", it.data());
    for (QStringList::Iterator it2 = urls.begin(); it2 != urls.end(); ++it2 )
    {
      urlList.append(FileInfo(KURL::fromPathOrURL(*it2)));
    }
    m_projectViews.insert(it.key(), urlList);
  }
  adjustViewActions();
}

#include "projectviewpart.moc"
