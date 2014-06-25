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
#include <interfaces/iuicontroller.h>
#include <kparts/mainwindow.h>
#include <kmessagebox.h>
#include <makefileresolver.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/ilanguagecontroller.h>

using namespace Cpp;

Cpp::AddCustomIncludePathAction::AddCustomIncludePathAction(KDevelop::IndexedString url, QString directive) {
  m_url = url;
  m_directive = directive;
}

//FIXME:
void Cpp::AddCustomIncludePathAction::execute() {
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
    auto project = KDevelop::ICore::self()->projectController()->findProjectForUrl(m_url.toUrl());

    if (!project) {
        addAction(KDevelop::IAssistantAction::Ptr(new OpenProjectForFileAssistant(m_url.toUrl())));
        addAction(KDevelop::IAssistantAction::Ptr(new AddCustomIncludePathAction(m_url, m_directive)));
    } else {
        addAction(KDevelop::IAssistantAction::Ptr(new OpenProjectConfigurationAction(project)));
    }
}

OpenProjectConfigurationAction::OpenProjectConfigurationAction(KDevelop::IProject* project)
    : m_project(project)
{}

QString OpenProjectConfigurationAction::description() const
{
    return i18n("Add Custom Include Path");
}

void OpenProjectConfigurationAction::execute()
{
    KDevelop::ICore::self()->projectController()->configureProject(m_project);
    emit executed(this);
}
