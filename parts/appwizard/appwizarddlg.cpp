/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2001 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qgrid.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtextview.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include "kdevmakefrontend.h"

#include "appwizardfactory.h"
#include "appwizardpart.h"
#include "appwizarddlg.h"
#include "filepropspage.h"
#include "misc.h"


AppWizardDialog::AppWizardDialog(AppWizardPart *part, QWidget *parent, const char *name)
    : AppWizardDialogBase(parent, name,true), m_pCurrentAppInfo(0)
{
  templates_listview->header()->hide();
  m_projectLocationWasChanged=false;

  KStandardDirs *dirs = AppWizardFactory::instance()->dirs();
  m_templateNames = dirs->findAllResources("apptemplates", QString::null, false, true);
  
  kdDebug(9010) << "Templates: " << endl;
  QString category;
  QString destDir;
  QStringList categories;
  QStringList::Iterator it;
  for (it = m_templateNames.begin(); it != m_templateNames.end(); ++it) {
    kdDebug(9010) << (*it) << endl;
    ApplicationInfo* pInfo = new ApplicationInfo();
    KConfig config(KGlobal::dirs()->findResource("apptemplates", *it));
    config.setGroup("General");
    pInfo->templateName = (*it);
    pInfo->name = config.readEntry("Name");
    pInfo->icon = config.readEntry("Icon");
    pInfo->comment = config.readEntry("Comment");
    pInfo->showFileAfterGeneration = config.readEntry("ShowFileAfterGeneration","");
    destDir = config.readEntry("DefaultDestinatonDir","HOMEDIR");
    destDir = destDir.replace(QRegExp("HOMEDIR"),QDir::homeDirPath());
    pInfo->defaultDestDir = destDir;
    category = config.readEntry("Category");
    // format category to a unique status
    if(category.right(1) == "/"){
      category.remove(category.length()-1,1); // remove /
    }
    if(category.left(1) != "/"){
      category.prepend("/"); // prepend /
    }

    categories.append(category);
    pInfo->category = category;
    m_appsInfo.append(pInfo);
  }
  categories.sort();
  for (it = categories.begin(); it != categories.end(); ++it) {
    insertCategoryIntoTreeView(*it);
  }
  ApplicationInfo* pInfo=0;
  QListViewItem* pItem=0;
  for(pInfo=m_appsInfo.first();pInfo!=0;pInfo=m_appsInfo.next()){
    pItem = m_categoryMap.find(pInfo->category);
    if(pItem !=0){
      pItem = new QListViewItem(pItem,pInfo->name);
      //      pItem->setPixmap(0, SmallIcon("resource"));
      pInfo->pItem = pItem;
    }
    else{
      kdDebug(9010) << "Error can't find category in categoryMap: " << pInfo->category << endl;
    }
  }


  QString author, email;
  AppWizardUtil::guessAuthorAndEmail(&author, &email);
  author_edit->setText(author);
  email_edit->setText(email);
  dest_edit->setText(QDir::homeDirPath()+"/");
  filetemplate_edit->setFont(KGlobalSettings::fixedFont());
  QFontMetrics fm(filetemplate_edit->fontMetrics());
  filetemplate_edit->setMinimumSize(fm.width("X")*81, fm.lineSpacing()*22);

    /*    //add a new page (fileprops)
	  QString projectname = "Test";
    FilePropsPage* m_sdi_fileprops_page = new FilePropsPage(this,"fileprops");
    QList<ClassFileProp>* props_temp = new QList<ClassFileProp>;
    ClassFileProp* prop = new ClassFileProp();
    prop->m_classname = projectname + "App";
    prop->m_headerfile = projectname.lower() + "app.h";
    prop->m_implfile = projectname.lower() + "app.cpp";
    prop->m_baseclass = "KMainWindow";
    prop->m_description = "The base class for the application window. It sets up the main window and reads the config file as well as providing a menubar, toolbar and statusbar. An instance of the View creates your center view, which is connected to the window's Doc object.";
    prop->m_change_baseclass = false;
    prop->m_key = "App";
    props_temp->append(prop);

    prop = new ClassFileProp();
    prop->m_classname = projectname + "View";
    prop->m_headerfile = projectname.lower() + "view.h";
    prop->m_implfile = projectname.lower() + "view.cpp";
    prop->m_baseclass = "QWidget";
    prop->m_description = "The View class provides the view widget for the App instance. The View instance inherits QWidget as a base class and represents the view object of a KMainWindow. As View is part of the document-view model, it needs a reference to the document object connected with it by the App class to manipulate and display the document structure provided by the Doc class.";
    prop->m_change_baseclass = true;
    prop->m_key = "View";
    props_temp->append(prop);

    prop = new ClassFileProp();
    prop->m_classname = projectname + "Doc";
    prop->m_headerfile = projectname.lower() + "doc.h";
    prop->m_implfile = projectname.lower() + "doc.cpp";
    prop->m_baseclass = "QObject";
    prop->m_description = "The Doc class provides a document object that can be used in conjunction with the classes App and View to create a document-view model for standard KDE applications based on KApplication and KMainWindow. Doc contains the methods for serialization of the document data from and to files";
    prop->m_change_baseclass = true;
    prop->m_key = "Doc";
    props_temp->append(prop);

    m_sdi_fileprops_page->setClassFileProps(*props_temp);
    */


    //    addPage(m_sdi_fileprops_page,"Class/File Properties");
    helpButton()->hide();

    connect( appname_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(textChanged()) );
    connect( dest_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(textChanged()) );
    connect( appname_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(projectNameChanged()) );
    connect( dest_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(projectLocationChanged()) );
    connect( author_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(textChanged()) );
    connect( version_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(textChanged()) );
    connect( license_combo, SIGNAL(activated(int)),
             this, SLOT(licenseChanged()) );
    connect( dest_button, SIGNAL(clicked()),
             this, SLOT(destButtonClicked()) );
    licenseChanged();
    tempFile = 0;
    m_part = part;
    nextButton()->setEnabled(!appname_edit->text().isEmpty());
}


AppWizardDialog::~AppWizardDialog()
{
    delete tempFile;
}


void AppWizardDialog::textChanged()
{

    bool invalid = !m_pCurrentAppInfo
        || appname_edit->text().isEmpty()
        || dest_edit->text().isEmpty()
        || author_edit->text().isEmpty()
        || version_edit->text().isEmpty();
    setFinishEnabled(fileHeadersPage, !invalid);
    nextButton()->setEnabled(!appname_edit->text().isEmpty());
}


void AppWizardDialog::licenseChanged()
{
    QString str =
        "/***************************************************************************\n"
        " *   Copyright (C) 2001 by $AUTHOR$                                        *\n"
        " *   $EMAIL$                                                               *\n"
        " *                                                                         *\n";

    QString author = author_edit->text();
    QString email = email_edit->text();
    str.replace(str.find("2001"), 4, QString::number(QDate::currentDate().year()));
    str.replace(str.find("$AUTHOR$                      "), QMIN(30, author.length()), author);
    str.replace(str.find("$EMAIL$                       "), QMIN(30, email.length()), email);

    switch (license_combo->currentItem())
        {
        case 0:
            str +=
                " *   This program is free software; you can redistribute it and/or modify  *\n"
                " *   it under the terms of the GNU General Public License as published by  *\n"
                " *   the Free Software Foundation; either version 2 of the License, or     *\n"
                " *   (at your option) any later version.                                   *\n";
            break;
        case 1:
            str +=
                " *   Permission is hereby granted, free of charge, to any person obtaining *\n"
                " *   a copy of this software and associated documentation files (the       *\n"
                " *   \"Software\"), to deal in the Software without restriction, including   *\n"
                " *   without limitation the rights to use, copy, modify, merge, publish,   *\n"
                " *   distribute, sublicense, and/or sell copies of the Software, and to    *\n"
                " *   permit persons to whom the Software is furnished to do so, subject to *\n"
                " *   the following conditions:                                             *\n"
                " *                                                                         *\n"
                " *   The above copyright notice and this permission notice shall be        *\n"
                " *   included in all copies or substantial portions of the Software.       *\n"
                " *                                                                         *\n"
                " *   THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,       *\n"
                " *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *\n"
                " *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*\n"
                " *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR     *\n"
                " *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, *\n"
                " *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR *\n"
                " *   OTHER DEALINGS IN THE SOFTWARE.                                       *\n";
            break;
        case 2:
            str +=
                " *   This program may be distributed under the terms of the Q Public       *\n"
                " *   License as defined by Trolltech AS of Norway and appearing in the     *\n"
                " *   file LICENSE.QPL included in the packaging of this file.              *\n"
                " *                                                                         *\n"
                " *   This program is distributed in the hope that it will be useful,       *\n"
                " *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *\n"
                " *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *\n";
            break;
        case 3:
            str +=
                " *   This program is free software; you can redistribute it and/or modify  *\n"
                " *   it under the terms of the GNU Library General Public License as       *\n"
                " *   published by the Free Software Foundation; either version 2 of the    *\n"
                " *   License, or (at your option) any later version.                       *\n";
        }

    str += " ***************************************************************************/\n";

    filetemplate_edit->setText(str);
}


void AppWizardDialog::accept()
{
    QFileInfo fi(dest_edit->text());
    if (fi.exists()) {
        KMessageBox::sorry(this, i18n("The directory you have chosen as location for "
                                      "the project already exists."));
        showPage(generalPage);
        dest_edit->setFocus();
        return;
    }

    if (!fi.dir().exists()) {
        // create dir if it doesn't exist
        KShellProcess p("/bin/sh");
        p.clearArguments();
        p << "mkdirhier";
        p << "\"" << dest_edit->text() << "\"";
        p.start(KProcess::Block,KProcess::AllOutput);
    }

    // if dir still does not exist
    if (!fi.dir().exists()) {
      KMessageBox::sorry(this, i18n("The directory above the chosen location does not exist and can not be created."));
      showPage(generalPage);
      dest_edit->setFocus();
      return;
    }

    QString appname = appname_edit->text();
    for (uint i=0; i < appname.length(); ++i)
        if (!appname[i].isLetterOrNumber()) {
            KMessageBox::sorry(this, i18n("Your application name should only contain letters and numbers, "
                                          "as it will be used as toplevel directory name."));
            showPage(generalPage);
            appname_edit->setFocus();
            return;
        }

    QString source, script;
    QFileInfo finfo(m_pCurrentAppInfo->templateName);
    QDir dir(finfo.dir());
    dir.cdUp();
    source = dir.absPath();
    script = dir.filePath("template-" + finfo.fileName() + "/script");

    QString license =
        (license_combo->currentItem()<4)? license_combo->currentText() : QString("Custom");

    QString licensefile;
    switch (license_combo->currentItem())
        {
        case 0: licensefile = "COPYING";     break;
        case 1: licensefile = "LICENSE.BSD"; break;
        case 2: licensefile = "LICENSE.QPL"; break;
        case 3: licensefile = "COPYING.LIB"; break;
        default: ;
        }

    if (!tempFile) {
        tempFile = new KTempFile();
	//        tempFile->setAutoDelete(true);
    }
    // KTempFile sucks
    QFile f;
    f.open(IO_WriteOnly, tempFile->handle());
    QTextStream temps(&f);
    temps << filetemplate_edit->text();
    f.flush();


    cmdline = "perl ";
    cmdline += script;
    cmdline += " --author=";
    cmdline += KShellProcess::quote(author_edit->text());
    cmdline += " --email=";
    cmdline +=  KShellProcess::quote(email_edit->text());
    cmdline += " --version=";
    cmdline +=  KShellProcess::quote(version_edit->text());
    cmdline += " --appname=";
    cmdline +=  KShellProcess::quote(appname_edit->text());
    cmdline += " --dest=";
    cmdline +=  KShellProcess::quote(dest_edit->text());
    cmdline += " --source=";
    cmdline +=  KShellProcess::quote(source);
    cmdline += " --license=";
    cmdline +=  KShellProcess::quote(license);
    cmdline += " --licensefile=";
    cmdline += KShellProcess::quote(licensefile);
    cmdline += " --filetemplate=";
    cmdline += KShellProcess::quote(tempFile->name());

    m_part->makeFrontend()->queueCommand(QString::null, cmdline);

    QWizard::accept();
}

void AppWizardDialog::insertCategoryIntoTreeView(QString completeCategoryPath){
  kdDebug(9010) << "TemplateCategory: " << completeCategoryPath << endl;
  QStringList categories = QStringList::split("/",completeCategoryPath);
  QStringList::Iterator it;
  QString category ="";
  QListViewItem* pItem=0;
  QListViewItem* pParentItem=0;
  for( it = categories.begin(); it != categories.end(); ++it ){
    category = category + "/"+ *it;
    pItem = m_categoryMap.find(category);
    if(pItem == 0){ // not found, create it
      if(pParentItem==0){
	pParentItem = new QListViewItem(templates_listview,*it);
      }
      else{
	pParentItem = new QListViewItem(pParentItem,*it);
      }
      pParentItem->setPixmap(0, SmallIcon("folder"));
      //pParentItem->setOpen(true);
      kdDebug(9010) << "Category: " << category << endl;
      m_categoryMap.insert(category,pParentItem);
    }
    else{
      pParentItem = pItem;
    }
  }
}


void AppWizardDialog::templatesTreeViewClicked(QListViewItem* pItem){
  ApplicationInfo* pInfo=0;
  for(pInfo=m_appsInfo.first();pInfo!=0;pInfo=m_appsInfo.next()){
    if(pInfo->pItem == pItem){
      if (!pInfo->icon.isEmpty()) {
        QFileInfo fi(pInfo->templateName);
        QDir dir(fi.dir());
        dir.cdUp();
        QPixmap pm;
        pm.load(dir.filePath("template-" + fi.fileName() + "/" + pInfo->icon));
        icon_label->setPixmap(pm);
      } else {
        icon_label->clear();
      }
      desc_textview->setText(pInfo->comment);
      dest_edit->setText(pInfo->defaultDestDir);
      m_pCurrentAppInfo = pInfo;
      m_projectLocationWasChanged = false;
      projectNameChanged(); // set the dest new
      return;
    }
  }
  icon_label->setPixmap(QPixmap());
  desc_textview->setText(QString());

}


void AppWizardDialog::destButtonClicked(){
  QString dir = KFileDialog::getExistingDirectory ( dest_edit->text(),this,
						    "Project Location" );
  if(!dir.isEmpty()){
    dest_edit->setText(dir);
  }
}

QString AppWizardDialog::getShowFileAfterGeneration(){
  if(m_pCurrentAppInfo !=0){
    if(m_pCurrentAppInfo->showFileAfterGeneration !=""){
      return dest_edit->text() + "/" + m_pCurrentAppInfo->showFileAfterGeneration;
    }
  }
  return "";
}
void AppWizardDialog::projectNameChanged(){
  if(!m_projectLocationWasChanged){
    if(m_pCurrentAppInfo !=0){
      dest_edit->setText(m_pCurrentAppInfo->defaultDestDir + "/" + QString(appname_edit->text()).lower());
    }
  }
  
}
void AppWizardDialog::projectLocationChanged(){
  if(dest_edit->hasFocus()){
    m_projectLocationWasChanged = true;
  }
}
#include "appwizarddlg.moc"
