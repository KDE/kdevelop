/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "unresolvedincludeassistant.h"
#include <klocalizedstring.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include "ui_custom_include_paths.h"
#include <interfaces/iuicontroller.h>
#include <kparts/mainwindow.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <includepathresolver.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/ilanguagecontroller.h>

using namespace Cpp;

Cpp::AddCustomIncludePathAction::AddCustomIncludePathAction(KDevelop::IndexedString url, QString directive) {
  m_url = url;
  m_directive = directive;
}

void Cpp::AddCustomIncludePathAction::execute() {
  Ui::CustomIncludePaths o;
  KDialog dialog(KDevelop::ICore::self()->uiController()->activeMainWindow());
  dialog.setInitialSize(QSize(600, 600));
  dialog.setButtons(KDialog::Ok | KDialog::Cancel);
  o.setupUi(dialog.mainWidget());
  
  KUrl current = m_url.toUrl().upUrl();
  
  o.storageDirectory->setUrl(current);
//   o.sourceDirectory->setUrl(current);
//   o.buildDirectory->setUrl(current);
  o.storageDirectory->setMode(KFile::Directory);
  o.sourceDirectory->setMode(KFile::Directory);
  o.buildDirectory->setMode(KFile::Directory);

  //Find old settings
  CppTools::CustomIncludePathsSettings oldSettings = CppTools::CustomIncludePathsSettings::findAndRead(current.toLocalFile());
  
  ///@todo Integrate with project manager, to only show useful options, and maybe merge them

  if(oldSettings.isValid()) {
    current = KUrl(oldSettings.storagePath);
    o.storageDirectory->setUrl(current);
    o.sourceDirectory->setUrl(KUrl(oldSettings.sourceDir));
    o.buildDirectory->setUrl(KUrl(oldSettings.buildDir));
    foreach(const QString& customPath, oldSettings.paths)
      if(!customPath.isEmpty())
        o.customIncludePaths->appendPlainText(customPath);
    if(!oldSettings.paths.isEmpty())
      o.customIncludePaths->appendPlainText("\n");
  }

  dialog.setWindowTitle(i18n("Setup Custom Include Paths"));
  int result = dialog.exec();
  if(result == QDialog::Accepted) {
    kDebug() << "storing settings";
    if((o.storageDirectory->text().isEmpty() || 
       (o.customIncludePaths->document()->toPlainText().trimmed().isEmpty() && (o.sourceDirectory->text().trimmed().isEmpty() ||o.buildDirectory->text().trimmed().isEmpty() || o.sourceDirectory->text().trimmed() == o.buildDirectory->text().trimmed())))  
      && oldSettings.isValid()) {
      kDebug() << "deleting settings";
      oldSettings.delete_();
    }else{
      if(oldSettings.isValid() && o.storageDirectory->url().isParentOf(KUrl(oldSettings.storagePath)))
        oldSettings.delete_(); //Delete old settings, when the settings are moved up in the hierarchy, so old settings won't shadow new ones
      oldSettings.sourceDir = o.sourceDirectory->url().toLocalFile();
      oldSettings.buildDir = o.buildDirectory->url().toLocalFile();
      oldSettings.paths.clear();
      foreach(const QString& customPath, o.customIncludePaths->document()->toPlainText().split("\n", QString::SkipEmptyParts))
        if(!customPath.isEmpty())
          oldSettings.paths << customPath;
      kDebug() << "saving, paths" << oldSettings.paths;
      kDebug() << "dirs" << oldSettings.sourceDir << oldSettings.buildDir;
        
      oldSettings.storagePath = o.storageDirectory->url().toLocalFile();
      if(!oldSettings.write()) {
        KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(), i18n("Failed to save custom include paths in directory: %1", oldSettings.storagePath));
      } else {
        emit executed(this);
      }
    }
  }else{
    kDebug() << "not accepted";
  }
  
  CppTools::IncludePathResolver::clearCache();
  
  //Trigger an update, so the user sees the progress
  KDevelop::ICore::self()->languageController()->backgroundParser()->addDocument(m_url);
}


QString Cpp::AddCustomIncludePathAction::description() const {
  return i18n("Add Custom Include Path");
}


Cpp::OpenProjectForFileAssistant::OpenProjectForFileAssistant(KUrl url) : m_url(url) {
}

void Cpp::OpenProjectForFileAssistant::execute() {
  KDevelop::ICore::self()->projectController()->openProjectForUrl(m_url);
  emit executed(this);
}

QString Cpp::OpenProjectForFileAssistant::description() const {
  return i18n("Open Project");
}

QString MissingIncludePathAssistant::title() const {
  return i18n("Include file \"%1\" not found", m_directive);
}

Cpp::MissingIncludePathAssistant::MissingIncludePathAssistant(KDevelop::IndexedString url, QString directive) {
  m_url = url;
  m_directive = directive;
}

void MissingIncludePathAssistant::createActions()
{
  MissingIncludePathAssistant* nonConst = const_cast<MissingIncludePathAssistant*>(this);

  KDevelop::IProject* project = KDevelop::ICore::self()->projectController()->findProjectForUrl(m_url.toUrl());

  if(!project)
    nonConst->addAction(KDevelop::IAssistantAction::Ptr(new OpenProjectForFileAssistant(m_url.toUrl())));

  nonConst->addAction(KDevelop::IAssistantAction::Ptr(new AddCustomIncludePathAction(m_url, m_directive)));
}

