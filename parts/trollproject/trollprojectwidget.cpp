/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2000-2001 by Trolltech AS.                              *
 *   info@trolltech.com                                                    *
 *                                                                         *
 *   Part of this file is taken from Qt Designer.                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "trollprojectwidget.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qsplitter.h>
#include <qptrstack.h>
#include <qtextstream.h>
#include <qprocess.h>
#include <qtimer.h>
#include <qdir.h>
#include <qinputdialog.h>
#include <qfiledialog.h>
#include <kdebug.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kregexp.h>
#include <kurl.h>
#include <qmessagebox.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "kdevtoplevel.h"
#include "domutil.h"
#include "trollprojectpart.h"

/**
 * Class ProjectViewItem
 */

ProjectItem::ProjectItem(Type type, QListView *parent, const QString &text)
    : QListViewItem(parent, text), typ(type)
{}


ProjectItem::ProjectItem(Type type, ProjectItem *parent, const QString &text)
    : QListViewItem(parent, text), typ(type)
{}


/**
 * Class SubprojectItem
 */

SubprojectItem::SubprojectItem(QListView *parent, const QString &text, const QString &scopeString)
    : ProjectItem(Subproject, parent, text)
{
    this->scopeString=scopeString;
    init();
}


SubprojectItem::SubprojectItem(SubprojectItem *parent, const QString &text,const QString &scopeString)
    : ProjectItem(Subproject, parent, text)
{
    this->scopeString=scopeString;
    init();
}

SubprojectItem::~SubprojectItem()
{
}


void SubprojectItem::init()
{
    groups.setAutoDelete(true);
    setPixmap(0, SmallIcon("folder"));
}


/**
 * Class GroupItem
 */

GroupItem::GroupItem(QListView *lv, GroupType type, const QString &text, const QString &scopeString)
    : ProjectItem(Group, lv, text)
{
    this->scopeString = scopeString;
    groupType = type;
    files.setAutoDelete(true);
    setPixmap(0, SmallIcon("tar"));
}


/**
 * Class FileItem
 */

FileItem::FileItem(QListView *lv, const QString &text)
    : ProjectItem(File, lv, text)
{
    setPixmap(0, SmallIcon("document"));
}


TrollProjectWidget::TrollProjectWidget(TrollProjectPart *part)
    : QVBox(0, "troll project widget")
{
    QSplitter *splitter = new QSplitter(Vertical, this);

    overview = new KListView(splitter, "project overview widget");
    overview->setResizeMode(QListView::LastColumn);
    overview->setSorting(-1);
    overview->header()->hide();
    overview->addColumn(QString::null);
    details = new KListView(splitter, "project details widget");
    details->setRootIsDecorated(true);
    details->setResizeMode(QListView::LastColumn);
    details->setSorting(-1);
    details->header()->hide();
    details->addColumn(QString::null);

    connect( overview, SIGNAL(selectionChanged(QListViewItem*)),
             this, SLOT(slotOverviewSelectionChanged(QListViewItem*)) );
    connect( overview, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotOverviewContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    //    connect( details, SIGNAL(selectionChanged(QListViewItem*)),
    //             this, SLOT(slotDetailsSelectionChanged(QListViewItem*)) );
    connect( details, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotDetailsExecuted(QListViewItem*)) );
    connect( details, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             this, SLOT(slotDetailsContextMenu(KListView*, QListViewItem*, const QPoint&)) );

    m_part = part;
    m_shownSubproject = 0;
}


TrollProjectWidget::~TrollProjectWidget()
{}


void TrollProjectWidget::openProject(const QString &dirName)
{
    SubprojectItem *item = new SubprojectItem(overview, "/","");
    item->subdir = dirName.right(dirName.length()-dirName.findRev('/'));
    item->path = dirName;
    item->m_RootBuffer = &(item->m_FileBuffer);
    parse(item);
    item->setOpen(true);
    overview->setSelected(item, true);
}


void TrollProjectWidget::closeProject()
{
    overview->clear();
    details->clear();
}


QStringList TrollProjectWidget::allSubprojects()
{
    int prefixlen = projectDirectory().length()+1;
    QStringList res;

    QListViewItemIterator it(overview);
    for (; it.current(); ++it) {
        if (it.current() == overview->firstChild())
            continue;
        QString path = static_cast<SubprojectItem*>(it.current())->path;
        res.append(path.mid(prefixlen));
    }

    return res;
}


QStringList TrollProjectWidget::allFiles()
{
    QStack<QListViewItem> s;
    QStringList res;

    for ( QListViewItem *item = overview->firstChild(); item;
          item = item->nextSibling()? item->nextSibling() : s.pop() ) {
        if (item->firstChild())
            s.push(item->firstChild());

        SubprojectItem *spitem = static_cast<SubprojectItem*>(item);
        QString path = spitem->path;
        QListIterator<GroupItem> tit(spitem->groups);
        for (; tit.current(); ++tit) {
            GroupItem::GroupType type = (*tit)->groupType;
            if (type == GroupItem::Sources || type == GroupItem::Headers) {
                QListIterator<FileItem> fit(tit.current()->files);
                for (; fit.current(); ++fit)
                    res.append(path + "/" + (*fit)->name);
            }
        }
    }

    return res;
}

QString TrollProjectWidget::projectDirectory()
{
    if (!overview->firstChild())
        return QString::null; //confused

    return static_cast<SubprojectItem*>(overview->firstChild())->path;
}


QString TrollProjectWidget::subprojectDirectory()
{
    if (!m_shownSubproject)
        return QString::null;

    return m_shownSubproject->path;
}



void TrollProjectWidget::slotOverviewSelectionChanged(QListViewItem *item)
{
    if (!item)
        return;
    cleanDetailView(m_shownSubproject);
    m_shownSubproject = static_cast<SubprojectItem*>(item);
    buildProjectDetailTree(m_shownSubproject,details);
}

void TrollProjectWidget::cleanDetailView(SubprojectItem *item)
{
  if (item)
  {
    // Remove all GroupItems and all of their children from the view
//    QListIterator<SubprojectItem> it(item->scopes);
//    for (; it.current(); ++it)
//    {
//      cleanDetailView(*it);
//      details->takeItem(*it);
//    }
    QListIterator<GroupItem> it1(item->groups);
    for (; it1.current(); ++it1) {
      // After AddTargetDialog, it can happen that an
      // item is not yet in the list view, so better check...
      if (it1.current()->parent())
      while ((*it1)->firstChild())
        (*it1)->takeItem((*it1)->firstChild());
      details->takeItem(*it1);
    }
  }

}

void TrollProjectWidget::buildProjectDetailTree(SubprojectItem *item,KListView *listviewControl)
{
  // Insert all GroupItems and all of their children into the view
  if (listviewControl)
  {
//    QListIterator<SubprojectItem> it1(item->scopes);
//    for (; it1.current(); ++it1)
//    {
//      listviewControl->insertItem(*it1);
//      buildProjectDetailTree(*it1,NULL);
//    }
    QListIterator<GroupItem> it2(item->groups);
    for (; it2.current(); ++it2)
    {
        listviewControl->insertItem(*it2);
        QListIterator<FileItem> it3((*it2)->files);
        for (; it3.current(); ++it3)
            (*it2)->insertItem(*it3);
        (*it2)->setOpen(true);
    }
  }
  else
  {
//    QListIterator<SubprojectItem> it1(item->scopes);
//    for (; it1.current(); ++it1)
//    {
//      item->insertItem(*it1);
//      buildProjectDetailTree(*it1,NULL);
//    }
    QListIterator<GroupItem> it2(item->groups);
    for (; it2.current(); ++it2)
    {
        item->insertItem(*it2);
        QListIterator<FileItem> it3((*it2)->files);
        for (; it3.current(); ++it3)
            (*it2)->insertItem(*it3);
        (*it2)->setOpen(true);
    }
  }
}

void TrollProjectWidget::slotDetailsExecuted(QListViewItem *item)
{
    if (!item)
        return;

    // We assume here that ALL items in both list views
    // are ProjectItem's
    ProjectItem *pvitem = static_cast<ProjectItem*>(item);
    if (pvitem->type() != ProjectItem::File)
        return;

    QString dirName = m_shownSubproject->path;
    FileItem *fitem = static_cast<FileItem*>(pvitem);
    m_part->partController()->editDocument(KURL(dirName + "/" + QString(fitem->name)));
    m_part->topLevel()->lowerView(this);
}


void TrollProjectWidget::slotOverviewContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;

    SubprojectItem *spitem = static_cast<SubprojectItem*>(item);

    KPopupMenu popup(i18n("Subproject %1").arg(item->text(0)), this);
    int idAddSubproject = popup.insertItem(SmallIcon("folder_new"),i18n("Add Subproject..."));
    int idBuild = popup.insertItem(SmallIcon("launch"),i18n("Build"));
    int idQmake = popup.insertItem(SmallIcon("launch"),i18n("Run qmake"));
    int r = popup.exec(p);

    QString relpath = spitem->path.mid(projectDirectory().length());
    if (r == idAddSubproject)
    {

      bool ok = FALSE;
      QString subdirname = QInputDialog::getText(
                        tr( "Add Subdir" ),
                        tr( "Please enter a name for the new subdir." ),
                        QLineEdit::Normal, QString::null, &ok, this );
      if ( ok && !subdirname.isEmpty() )
      {
        QDir dir(projectDirectory()+relpath);
        if (!dir.mkdir(subdirname))
        {
          KMessageBox::error(this,i18n("Failed to create subdirectory. "
                                       "Do you have write permission "
                                       "in the projectfolder?" ));
          return;
        }
        spitem->subdirs.append(subdirname);
        updateProjectFile(spitem);
        SubprojectItem *newitem = new SubprojectItem(spitem, subdirname,"");
        newitem->subdir = subdirname;
        newitem->path = spitem->path + "/" + subdirname;

      }
      else
        return;
    }
    else if (r == idBuild)
    {
        m_part->startMakeCommand(projectDirectory() + relpath, QString::fromLatin1(""));
        m_part->topLevel()->lowerView(this);
    }
    else if (r == idQmake)
    {
        m_part->startQMakeCommand(projectDirectory() + relpath);
        m_part->topLevel()->lowerView(this);
    }
}

void TrollProjectWidget::updateProjectFile(QListViewItem *item)
{
  SubprojectItem *spitem = static_cast<SubprojectItem*>(item);
  QString relpath = m_shownSubproject->path.mid(projectDirectory().length());
  FileBuffer *subBuffer=m_shownSubproject->m_RootBuffer->getSubBuffer(spitem->scopeString);
  subBuffer->removeValues("SUBDIRS");
  subBuffer->setValues("SUBDIRS",spitem->subdirs,4);
  subBuffer->removeValues("SOURCES");
  subBuffer->setValues("SOURCES",spitem->sources,4);
  subBuffer->removeValues("HEADERS");
  subBuffer->setValues("HEADERS",spitem->headers,4);
  subBuffer->removeValues("FORMS");
  subBuffer->setValues("FORMS",spitem->forms,4);
  subBuffer->removeValues("INTERFACES");
  subBuffer->setValues("INTERFACES",spitem->interfaces,4);
  m_shownSubproject->m_RootBuffer->saveBuffer(projectDirectory()+relpath+"/"+m_shownSubproject->subdir+".pro");
}

void TrollProjectWidget::addFileToCurrentSubProject(GroupItem *titem,QString &filename)
{
  FileItem *fitem = createFileItem(filename);
  titem->files.append(fitem);
  switch (titem->groupType)
  {
    case GroupItem::Interfaces:
      titem->owner->interfaces.append(filename);
      break;
    case GroupItem::Sources:
      titem->owner->sources.append(filename);
      break;
    case GroupItem::Headers:
      titem->owner->headers.append(filename);
      break;
    case GroupItem::Forms:
      titem->owner->forms.append(filename);
      break;
  }
}


void TrollProjectWidget::slotDetailsContextMenu(KListView *, QListViewItem *item, const QPoint &p)
{
    if (!item)
        return;

    ProjectItem *pvitem = static_cast<ProjectItem*>(item);
    if (pvitem->type() == ProjectItem::Group) {

        GroupItem *titem = static_cast<GroupItem*>(pvitem);
        QString title,ext;
        switch (titem->groupType) {
        case GroupItem::Interfaces:
            title = i18n("Interfaces");
            ext = "*.*";
            break;
        case GroupItem::Sources:
            title = i18n("Sources");
            ext = "*.cpp *.c";
            break;
        case GroupItem::Headers:
            title = i18n("Headers");
            ext = "*.h *.hpp";
            break;
        case GroupItem::Forms:
            title = i18n("Forms");
            ext = "*.ui";
            break;
        default: ;
        }

        KPopupMenu popup(title, this);
        int idInsExistingFile = popup.insertItem(SmallIconSet("fileopen"),i18n("Insert existing files..."));
        int idInsNewFile = popup.insertItem(SmallIconSet("filenew"),i18n("Insert new file..."));
        int r = popup.exec(p);
        QString relpath = m_shownSubproject->path.mid(projectDirectory().length());
        if (r == idInsExistingFile)
        {
          QFileDialog *dialog = new QFileDialog(projectDirectory()+relpath,
                                                title + " ("+ext+")",
                                                this,
                                                "Insert existing "+ title,
                                                TRUE);
          dialog->setMode(QFileDialog::ExistingFiles);
          dialog->exec();
          QStringList files = dialog->selectedFiles();
          for (unsigned int i=0;i<files.count();i++)
          {
            // Copy selected files to current subproject folder
            QProcess *proc = new QProcess( this );
            proc->addArgument( "cp" );
            proc->addArgument( "-f" );
            proc->addArgument( files[i] );
            proc->addArgument( projectDirectory()+relpath );
            proc->start();
            QString filename = files[i].right(files[i].length()-files[i].findRev('/')-1);
            // and add them to the filelist
            addFileToCurrentSubProject(titem,filename);
          }
          // Update project file
          updateProjectFile(titem->owner);
          // Update subprojectview
          slotOverviewSelectionChanged(m_shownSubproject);
        }
        if (r == idInsNewFile)
        {
          bool ok = FALSE;
          QString filename = QInputDialog::getText(
                            tr( "Insert New File"),
                            tr( "Please enter a name for the new file." ),
                            QLineEdit::Normal, QString::null, &ok, this );
          if ( ok && !filename.isEmpty() )
          {
            QFile newfile(projectDirectory()+relpath+'/'+filename);
            if (!newfile.open(IO_WriteOnly))
            {
              KMessageBox::error(this,i18n("Failed to create new file. "
                                           "Do you have write permission "
                                           "in the projectfolder?" ));
              return;
            }
            addFileToCurrentSubProject(titem,filename);
            updateProjectFile(titem->owner);
            slotOverviewSelectionChanged(m_shownSubproject);
          }
        }

    } else if (pvitem->type() == ProjectItem::File) {

        FileItem *fitem = static_cast<FileItem*>(pvitem);

        KPopupMenu popup(i18n("File: %1").arg(fitem->name), this);
        int idRemoveFile = popup.insertItem(i18n("Remove File..."));

        FileContext context(m_shownSubproject->path + "/" + fitem->name, false);
        m_part->core()->fillContextMenu(&popup, &context);

        int r = popup.exec(p);
        if (r == idRemoveFile)
            removeFile(m_shownSubproject, fitem);

    }
}


void TrollProjectWidget::removeFile(SubprojectItem *spitem, FileItem *fitem)
{
    GroupItem *gitem = static_cast<GroupItem*>(fitem->parent());

    emitRemovedFile(spitem->path + "/" + fitem->text(0));
    switch (gitem->groupType)
    {
      case GroupItem::Interfaces:
        spitem->interfaces.remove(fitem->text(0));
        break;
      case GroupItem::Sources:
        spitem->sources.remove(fitem->text(0));
        break;
      case GroupItem::Headers:
        spitem->headers.remove(fitem->text(0));
        break;
      case GroupItem::Forms:
        spitem->forms.remove(fitem->text(0));
        break;
      default: ;
    }
    gitem->files.remove(fitem);
    updateProjectFile(m_shownSubproject);
}


GroupItem *TrollProjectWidget::createGroupItem(GroupItem::GroupType groupType, const QString &name,const QString &scopeString)
{
    // Workaround because for QListView not being able to create
    // items without actually inserting them
    GroupItem *titem = new GroupItem(overview, groupType, name,scopeString);
    overview->takeItem(titem);

    return titem;
}


FileItem *TrollProjectWidget::createFileItem(const QString &name)
{
    FileItem *fitem = new FileItem(overview, name);
    overview->takeItem(fitem);
    fitem->name = name;

    return fitem;
}

void TrollProjectWidget::emitAddedFile(const QString &fileName)
{
    emit m_part->addedFileToProject(fileName);
}


void TrollProjectWidget::emitRemovedFile(const QString &fileName)
{
    emit m_part->removedFileFromProject(fileName);
}


void TrollProjectWidget::parseScope(SubprojectItem *item, QString scopeString, FileBuffer *buffer)
{
    if (scopeString!="")
    {
      QStringList scopeNames = QStringList::split(':',scopeString);
      SubprojectItem *sitem;
      sitem = new SubprojectItem(item, scopeNames[scopeNames.count()-1],scopeString);
      sitem->path = item->path;
      sitem->m_RootBuffer = buffer;
      sitem->subdir = item->subdir;
      item->scopes.append(sitem);
      item=sitem;
    }

    QString values;
    FileBuffer *subBuffer = buffer->getSubBuffer(scopeString);
    values = subBuffer->getValues("INTERFACES");
    if (values != "")
      item->interfaces = QStringList::split(' ',values);

    values = subBuffer->getValues("FORMS");
    if (values != "")
      item->forms = QStringList::split(' ',values);

    values = subBuffer->getValues("SOURCES");
    if (values != "")
      item->sources = QStringList::split(' ',values);

    values = subBuffer->getValues("HEADERS");
    if (values != "")
      item->headers = QStringList::split(' ',values);

    // Create list view items
    GroupItem *titem = createGroupItem(GroupItem::Interfaces, "INTERFACES",scopeString);
    item->groups.append(titem);
    titem->owner = item;
    if (!item->interfaces.isEmpty()) {
        QStringList l = item->interfaces;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            titem->files.append(fitem);
        }
    }
    titem = createGroupItem(GroupItem::Forms, "FORMS",scopeString);
    item->groups.append(titem);
    titem->owner = item;
    if (!item->forms.isEmpty()) {
        QStringList l = item->forms;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            titem->files.append(fitem);
        }
    }
    titem = createGroupItem(GroupItem::Sources, "SOURCES",scopeString);
    item->groups.append(titem);
    titem->owner = item;
    if (!item->sources.isEmpty()) {
        QStringList l = item->sources;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            titem->files.append(fitem);
        }
    }
    titem = createGroupItem(GroupItem::Headers, "HEADERS",scopeString);
    titem->owner = item;
    item->groups.append(titem);
    if (!item->headers.isEmpty()) {
        QStringList l = item->headers;
        QStringList::Iterator it;
        for (it = l.begin(); it != l.end(); ++it) {
            FileItem *fitem = createFileItem(*it);
            titem->files.append(fitem);
        }
    }
    QStringList childScopes = subBuffer->getChildScopeNames();
    for (unsigned int i=0; i<childScopes.count();i++)
      parseScope(item,scopeString+(scopeString!="" ? ":" : "")+childScopes[i],buffer);
}

void TrollProjectWidget::parse(SubprojectItem *item)
{
    QFileInfo fi(item->path);
    QString proname = item->path + "/" + fi.baseName() + ".pro";
    kdDebug(9024) << "Parsing " << proname << endl;


    item->m_FileBuffer.bufferFile(proname);
    item->m_FileBuffer.handleScopes();

    parseScope(item,"",&(item->m_FileBuffer));
    QString values = item->m_FileBuffer.getValues("SUBDIRS");

    if (values != "")
    {
        QStringList lst = QStringList::split( ' ', values );
        item->subdirs = lst;
        QStringList::Iterator it;
        for (it = lst.begin(); it != lst.end(); ++it) {
            SubprojectItem *newitem = new SubprojectItem(item, (*it),"");
            newitem->subdir = *it;
            newitem->m_RootBuffer = &(newitem->m_FileBuffer);
            newitem->path = item->path + "/" + (*it);
            parse(newitem);
        }
    }

}

#include "trollprojectwidget.moc"
