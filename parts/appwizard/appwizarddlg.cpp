/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
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

#include "appwizarddlg.h"

#include <qvbox.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qgrid.h>
#include <qheader.h>
#include <qlistview.h>
#include <qmap.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtextview.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvalidator.h>
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
#include <kfile.h>
#include <kapplication.h>

#include <ktrader.h>
#include <kparts/componentfactory.h>

#include "kdevversioncontrol.h"
#include "kdevmakefrontend.h"
#include "appwizardfactory.h"
#include "appwizardpart.h"
#include "filepropspage.h"
#include "misc.h"


AppWizardDialog::AppWizardDialog(AppWizardPart *part, QWidget *parent, const char *name)
    : AppWizardDialogBase(parent, name,true), m_pCurrentAppInfo(0)
{
	kdDebug( 9000 ) << "  ** AppWizardDialog::AppWizardDialog()" << endl;

    connect( this, SIGNAL( selected( const QString & ) ), this, SLOT( pageChanged() ) );

    helpButton()->hide();
    templates_listview->header()->hide();

    m_pathIsValid=false;
    m_part = part;
    m_projectLocationWasChanged=false;
    m_appsInfo.setAutoDelete(true);
    m_tempFiles.setAutoDelete(true);

    KConfig *config = kapp->config();
    config->setGroup("General Options");
    QString defaultProjectsDir = config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()+"/");

    KStandardDirs *dirs = AppWizardFactory::instance()->dirs();
    QStringList m_templateNames = dirs->findAllResources("apptemplates", QString::null, false, true);

    kdDebug(9010) << "Templates: " << endl;
    QStringList categories;

    QStringList::Iterator it;
    for (it = m_templateNames.begin(); it != m_templateNames.end(); ++it) {
        kdDebug(9010) << (*it) << endl;
        KConfig config(KGlobal::dirs()->findResource("apptemplates", *it));
        config.setGroup("General");

        ApplicationInfo *info = new ApplicationInfo;
        info->templateName = (*it);
        info->name = config.readEntry("Name");
        info->icon = config.readEntry("Icon");
        info->comment = config.readEntry("Comment");
        info->fileTemplates = config.readEntry("FileTemplates");
        info->openFilesAfterGeneration = config.readListEntry("ShowFilesAfterGeneration");
        QString destDir = config.readPathEntry("DefaultDestinatonDir", defaultProjectsDir);
        destDir.replace(QRegExp("HOMEDIR"), QDir::homeDirPath());
        info->defaultDestDir = destDir;
        QString category = config.readEntry("Category");
        // format category to a unique status
        if (category.right(1) == "/")
            category.remove(category.length()-1, 1); // remove /
        if (category.left(1) != "/")
            category.prepend("/"); // prepend /
        categories.append(category);
        info->category = category;
        m_appsInfo.append(info);
    }

    // Insert categories into list view
    categories.sort();
    for (it = categories.begin(); it != categories.end(); ++it)
        insertCategoryIntoTreeView(*it);

    // Insert items into list view
    QPtrListIterator<ApplicationInfo> ait(m_appsInfo);
    for (; ait.current(); ++ait) {
        QListViewItem *item = m_categoryMap.find(ait.current()->category);
        if (item)
            item = new QListViewItem(item, ait.current()->name);
        else
            kdDebug(9010) << "Error can't find category in categoryMap: "
                          << ait.current()->category << endl;
        ait.current()->item = item;
    }

    QString author, email;
    AppWizardUtil::guessAuthorAndEmail(&author, &email);
    author_edit->setText(author);
    email_edit->setText(email);
    QToolTip::add( dest_edit->button(), i18n("Choose projects directory") );
    dest_edit->setURL(defaultProjectsDir);
    dest_edit->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);

    /*    //add a new page (fileprops)
	  QString projectname = "Test";
    FilePropsPage* m_sdi_fileprops_page = new FilePropsPage(this,"fileprops");
    QPtrList<ClassFileProp>* props_temp = new QPtrList<ClassFileProp>;
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
    loadVcs();

    //    addPage(m_sdi_fileprops_page,"Class/File Properties");

    //    licenseChanged();

    setNextEnabled(generalPage, false);
//    nextButton()->setEnabled(!appname_edit->text().isEmpty());

//    QRegExp appname_regexp ("[a-zA-Z][a-zA-Z0-9_]*"); //Non-Unicode version
    /* appname will start with a letter, and will contain letters,
       digits or underscores. */
    QRegExp appname_regexp ("[a-zA-Z][a-zA-Z0-9_]*");
    // How about names like "__" or "123" for project name? Are they legal?
    QRegExpValidator *appname_edit_validator;
    appname_edit_validator = new QRegExpValidator (appname_regexp,
                                                   appname_edit,
                                                   "AppNameValidator");
    appname_edit->setValidator(appname_edit_validator);
}

AppWizardDialog::~AppWizardDialog()
{}

void AppWizardDialog::loadVcs()
{
    m_vcsForm = new VcsForm();

    int i=0;
    m_vcsForm->combo->insertItem( i18n("no version control system", "None"), i );
    m_vcsForm->stack->addWidget( 0, i++ );

    // We query for all vcs plugins for KDevelop
    QStringList availableVcs = m_part->registeredVersionControls();

    for(QStringList::const_iterator it( availableVcs.begin() ); it != availableVcs.end(); ++it)
    {
        KDevVersionControl *vcs = m_part->versionControlByName( (*it) );
        QString vcsName = vcs->uid();

        QWidget *newProjectWidget = vcs->newProjectWidget( m_vcsForm->stack );
        if (newProjectWidget) {
            m_vcsForm->combo->insertItem( vcsName, i );
            m_vcsForm->stack->addWidget( newProjectWidget, i++ );
        }
        else
        {
            kdDebug( 9000 ) << "  ** Warning: VCS has not widget. Skipping. " << endl;
        }
    }

    addPage( m_vcsForm, i18n("Version Control System") );
}


void AppWizardDialog::textChanged()
{
    licenseChanged();

    bool invalid = !m_pCurrentAppInfo
        || appname_edit->text().isEmpty()
        || !m_pathIsValid
        || author_edit->text().isEmpty()
        || version_edit->text().isEmpty();
    setFinishEnabled(m_lastPage, !invalid);
    nextButton()->setEnabled(!invalid);

}


void AppWizardDialog::licenseChanged()
{
    QString str =
        "/***************************************************************************\n"
        " *   Copyright (C) %1 by %2   *\n"
        " *   %3   *\n"
        " *                                                                         *\n";

    QString author = author_edit->text();
    QString email = email_edit->text();

    str = str.arg(QDate::currentDate().year()).arg(author.left(45),-45).arg(email.left(67),-67);

    switch (license_combo->currentItem())
        {
        case 0:
            str +=
                " *   This program is free software; you can redistribute it and/or modify  *\n"
                " *   it under the terms of the GNU General Public License as published by  *\n"
                " *   the Free Software Foundation; either version 2 of the License, or     *\n"
                " *   (at your option) any later version.                                   *\n"
                " *                                                                         *\n"
                " *   This program is distributed in the hope that it will be useful,       *\n"
                " *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *\n"
                " *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *\n"
                " *   GNU General Public License for more details.                          *\n"
                " *                                                                         *\n"
                " *   You should have received a copy of the GNU General Public License     *\n"
                " *   along with this program; if not, write to the                         *\n"
                " *   Free Software Foundation, Inc.,                                       *\n"
                " *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *\n";
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
                " *   License, or (at your option) any later version.                       *\n"
                " *                                                                         *\n"
                " *   This program is distributed in the hope that it will be useful,       *\n"
                " *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *\n"
                " *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *\n"
                " *   GNU General Public License for more details.                          *\n"
                " *                                                                         *\n"
                " *   You should have received a copy of the GNU Library General Public     *\n"
                " *   License along with this program; if not, write to the                 *\n"
                " *   Free Software Foundation, Inc.,                                       *\n"
                " *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *\n";
	    break;
        case 4:
            str +=
                " *   This program is free software; you can redistribute it and/or modify  *\n"
                " *   it under the terms of the GNU General Public License as published by  *\n"
                " *   the Free Software Foundation; either version 2 of the License, or     *\n"
                " *   (at your option) any later version.                                   *\n"
                " *                                                                         *\n"
                " *   This program is distributed in the hope that it will be useful,       *\n"
                " *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *\n"
                " *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *\n"
                " *   GNU General Public License for more details.                          *\n"
                " *                                                                         *\n"
                " *   You should have received a copy of the GNU General Public License     *\n"
                " *   along with this program; if not, write to the                         *\n"
                " *   Free Software Foundation, Inc.,                                       *\n"
                " *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *\n"
                " *                                                                         *\n"
                " *   In addition, as a special exception, the copyright holders give       *\n"
                " *   permission to link the code of this program with any edition of       *\n"
                " *   the Qt library by Trolltech AS, Norway (or with modified versions     *\n"
                " *   of Qt that use the same license as Qt), and distribute linked         *\n"
                " *   combinations including the two.  You must obey the GNU General        *\n"
                " *   Public License in all respects for all of the code used other than    *\n"
                " *   Qt.  If you modify this file, you may extend this exception to        *\n"
                " *   your version of the file, but you are not obligated to do so.  If     *\n"
                " *   you do not wish to do so, delete this exception statement from        *\n"
                " *   your version.                                                         *\n";
        }

    str += " ***************************************************************************/\n";

    QValueList<AppWizardFileTemplate>::Iterator it;
    for (it = m_fileTemplates.begin(); it != m_fileTemplates.end(); ++it) {
        QString style = (*it).style;
        QMultiLineEdit *edit = (*it).edit;

        QString text;
        if (style == "CStyle") {
            text = str;
        } else if (style == "PStyle") {
            text = str;
            text.replace(QRegExp("/\\**\n \\*"), "{\n  ");
            text.replace(QRegExp("\\*\n \\*"), " \n  ");
            text.replace(QRegExp(" *\\**/\n"), "}\n");
        } else if (style == "AdaStyle") {
            text = str;
            text.replace(QRegExp("/\\*"), "--");
            text.replace(QRegExp(" \\*"), "--");
            text.replace(QRegExp("\\*/"), "*");
        } else if (style == "ShellStyle") {
            text = str;
            text.replace(QRegExp("\\*|/"), "#");
            text.replace(QRegExp("\n ##"), "\n##");
            text.replace(QRegExp("\n #"), "\n# ");
        }

        edit->setText(text);
    }
}


void AppWizardDialog::accept()
{
    QFileInfo fi(finalLoc_label->text());
    // check /again/ whether the dir already exists; maybe users create it in the meantime
    if (fi.exists()) {
        KMessageBox::sorry(this, i18n("The directory you have chosen as the location for "
                                      "the project already exists."));
        showPage(generalPage);
        appname_edit->setFocus();
        projectLocationChanged();
        return;
    }

    if (!fi.dir().exists()) {
        // create dir if it doesn't exist
        KShellProcess p("/bin/sh");
        p.clearArguments();
        p << "mkdirhier";
        p << KShellProcess::quote(finalLoc_label->text());
        p.start(KProcess::Block,KProcess::AllOutput);
    }

    // if dir still does not exist
    if (!fi.dir().exists()) {
      KMessageBox::sorry(this, i18n("The directory above the chosen location does not exist and cannot be created."));
      showPage(generalPage);
      dest_edit->setFocus();
      return;
    }

/*  // this piece of code is rendered useless by the QValidator
    QString appname = appname_edit->text();
    for (uint i=0; i < appname.length(); ++i)
        if (!appname[i].isLetterOrNumber() && appname[i] != '_') {
            KMessageBox::sorry(this, i18n("Your application name should only contain letters and numbers, "
                                          "as it will be used as the top level directory name."));
            showPage(generalPage);
            appname_edit->setFocus();
            return;
        }
*/

    QString source, script;
    QFileInfo finfo(m_pCurrentAppInfo->templateName);
    QDir dir(finfo.dir());
    dir.cdUp();
    source = dir.absPath();
    script = dir.filePath("template-" + finfo.fileName() + "/script");

    QString license =
        (license_combo->currentItem()<4)? license_combo->currentText() : i18n("Custom");

    QString licensefile;
    switch (license_combo->currentItem())
        {
        case 0: licensefile = "COPYING";     break;
        case 1: licensefile = "LICENSE.BSD"; break;
        case 2: licensefile = "LICENSE.QPL"; break;
        case 3: licensefile = "COPYING.LIB"; break;
        case 4: licensefile = "COPYING";     break;
        default: ;
        }

    QStringList templateFiles;
    QValueList<AppWizardFileTemplate>::Iterator it;
    for (it = m_fileTemplates.begin(); it != m_fileTemplates.end(); ++it) {
        KTempFile *tempFile = new KTempFile();
        m_tempFiles.append(tempFile);

        QFile f;
        f.open(IO_WriteOnly, tempFile->handle());
        QTextStream temps(&f);
        temps << (*it).edit->text();
        f.flush();

        templateFiles << (*it).suffix;
        templateFiles << tempFile->name();
    }

    m_cmdline = "perl ";
    m_cmdline += script;
    m_cmdline += " --author=";
    m_cmdline += KShellProcess::quote(QString::fromLocal8Bit(author_edit->text().utf8()));
    m_cmdline += " --email=";
    m_cmdline +=  KShellProcess::quote(email_edit->text());
    m_cmdline += " --version=";
    m_cmdline +=  KShellProcess::quote(version_edit->text());
    m_cmdline += " --appname=";
    m_cmdline +=  KShellProcess::quote(appname_edit->text());
    m_cmdline += " --dest=";
    m_cmdline +=  KShellProcess::quote(finalLoc_label->text());
    m_cmdline += " --source=";
    m_cmdline +=  KShellProcess::quote(source);
    m_cmdline += " --license=";
    m_cmdline +=  KShellProcess::quote(license);
    m_cmdline += " --licensefile=";
    m_cmdline += KShellProcess::quote(licensefile);
    m_cmdline += " --filetemplates=";
    m_cmdline += KShellProcess::quote(templateFiles.join(","));

    m_part->makeFrontend()->queueCommand(QString::null, m_cmdline);

    if (m_vcsForm->stack->id(m_vcsForm->stack->visibleWidget())) {
        KDevVersionControl* pVC = m_part->versionControlByName( m_vcsForm->combo->currentText() );
        if (pVC) {
			kdDebug( 9000 ) << "Creating new project with selected VCS ..." << endl;
            pVC->createNewProject(finalLoc_label->text());
        }
		else
		{
			kdDebug( 9000 ) << "Could not grab the selected VCS: " << m_vcsForm->combo->currentText() << endl;
		}
    }

    QWizard::accept();
}


void AppWizardDialog::templatesTreeViewClicked(QListViewItem *item)
{
    // Delete old file template pages
    while (!m_fileTemplates.isEmpty()) {
        QMultiLineEdit *edit = m_fileTemplates.first().edit;
        removePage(edit);
        delete edit;
        m_fileTemplates.remove(m_fileTemplates.begin());
    }
    m_lastPage = 0;

    ApplicationInfo *info = templateForItem(item);
    if (info) {
        m_pCurrentAppInfo = info;
        if (!info->icon.isEmpty()) {
            QFileInfo fi(info->templateName);
            QDir dir(fi.dir());
            dir.cdUp();
            QPixmap pm;
            pm.load(dir.filePath("template-" + fi.fileName() + "/" + info->icon));
            icon_label->setPixmap(pm);
        } else {
            icon_label->clear();
        }
        desc_textview->setText(info->comment);
//        dest_edit->setURL(info->defaultDestDir);
        m_projectLocationWasChanged = false;
        //projectNameChanged(); // set the dest new

        // Create new file template pages
        QStringList l = QStringList::split(",", info->fileTemplates);
        QStringList::ConstIterator it = l.begin();
        while (it != l.end()) {
            AppWizardFileTemplate fileTemplate;
            fileTemplate.suffix = *it;
            ++it;
            if (it != l.end()) {
                fileTemplate.style = *it;
                ++it;
            } else
                fileTemplate.style = "";

            QMultiLineEdit *edit = new QMultiLineEdit(this);
            edit->setWordWrap(QTextEdit::NoWrap);
            edit->setFont(KGlobalSettings::fixedFont());
            if (it == l.end())
                m_lastPage = edit;
            fileTemplate.edit = edit;
            addPage(edit, i18n("Template for .%1 Files").arg(fileTemplate.suffix));
            m_fileTemplates.append(fileTemplate);
        }
        // licenseChanged(); // update template editors
        textChanged(); // calls licenseChanged() && update Next button state
    } else {
        m_pCurrentAppInfo=0;
        icon_label->clear();
        desc_textview->clear();
        nextButton()->setEnabled(false);
    }
}


void AppWizardDialog::destButtonClicked(const QString& dir)
{
    if(!dir.isEmpty()) {

        // set new location as default project dir?
        KConfig *config = kapp->config();
        config->setGroup("General Options");
        QDir defPrjDir( config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()) );
        QDir newDir (dir);
        kdDebug(9010) << "DevPrjDir == newdir?: " << defPrjDir.absPath() << " == " << newDir.absPath() << endl;
        if (defPrjDir != newDir) {
            if (KMessageBox::questionYesNo(this, i18n("Set default project location to: ") + newDir.absPath() + "?",
                                           i18n("New Project")) == KMessageBox::Yes)
            {
                config->writePathEntry("DefaultProjectsDir", newDir.absPath() + "/");
                config->sync();
            }
        }
    }
}


void AppWizardDialog::projectNameChanged()
{
    // Location was already edited by hand => don't change
}


void AppWizardDialog::projectLocationChanged()
{
  // Jakob Simon-Gaarde: Got tired of the anoying bug with the appname/location confussion.
  // This version insures WYSIWYG and checks pathvalidity
  finalLoc_label->setText(dest_edit->url() + (dest_edit->url().right(1)=="/" ? "":"/") + appname_edit->text().lower());
  QDir qd(dest_edit->url());
  QFileInfo fi(dest_edit->url() + "/" + appname_edit->text().lower());
  if (!qd.exists() || appname_edit->displayText().isEmpty()||fi.exists())
  {
    if (!fi.exists() || appname_edit->displayText().isEmpty()) {
      finalLoc_label->setText(finalLoc_label->text() + i18n("invalid location", " (invalid)"));
    } else {
      finalLoc_label->setText(finalLoc_label->text() + i18n(" (dir/file already exist)"));
    }
    m_pathIsValid=false;
  } else {
    m_pathIsValid=true;
  }
    bool invalid = !m_pCurrentAppInfo
       || appname_edit->text().isEmpty()
       || !m_pathIsValid
       || author_edit->text().isEmpty()
       || version_edit->text().isEmpty();
    setFinishEnabled(m_lastPage, !invalid);
    nextButton()->setEnabled(!invalid);
}


void AppWizardDialog::insertCategoryIntoTreeView(const QString &completeCategoryPath)
{
    kdDebug(9010) << "TemplateCategory: " << completeCategoryPath << endl;
    QStringList categories = QStringList::split("/", completeCategoryPath);
    QString category ="";
    QListViewItem* pParentItem=0;

    QStringList::ConstIterator it;
    for (it = categories.begin(); it != categories.end(); ++it) {
        category = category + "/" + *it;
        QListViewItem *item = m_categoryMap.find(category);
        if (!item) { // not found, create it
            if (!pParentItem)
                pParentItem = new QListViewItem(templates_listview,*it);
            else
                pParentItem = new QListViewItem(pParentItem,*it);

            pParentItem->setPixmap(0, SmallIcon("folder"));
            //pParentItem->setOpen(true);
            kdDebug(9010) << "Category: " << category << endl;
            m_categoryMap.insert(category,pParentItem);
        } else {
            pParentItem = item;
        }
    }
}


ApplicationInfo *AppWizardDialog::templateForItem(QListViewItem *item)
{
    QPtrListIterator<ApplicationInfo> it(m_appsInfo);
    for (; it.current(); ++it)
        if (it.current()->item == item)
            return it.current();

    return 0;
}

QStringList AppWizardDialog::getFilesToOpenAfterGeneration()
{
    for ( QStringList::Iterator it = m_pCurrentAppInfo->openFilesAfterGeneration.begin();
          it != m_pCurrentAppInfo->openFilesAfterGeneration.end(); ++it ) {
        (*it).replace("APPNAMEUC", getProjectName().upper());
        (*it).replace("APPNAMELC", getProjectName().lower());
        (*it).replace("APPNAME", getProjectName());
    }
    return m_pCurrentAppInfo->openFilesAfterGeneration;
}

void AppWizardDialog::pageChanged()
{
	kdDebug(9010) << "AppWizardDialog::pageChanged()" << endl;
	projectLocationChanged();
}

#include "appwizarddlg.moc"

