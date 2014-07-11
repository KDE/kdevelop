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
#include <custom-definesandincludes/idefinesandincludesmanager.h>

using namespace Cpp;

namespace
{
void openConfigurationPage(const QString& path)
{
    KDevelop::IDefinesAndIncludesManager::manager()->openConfigurationDialog(path);
}
}

Cpp::AddCustomIncludePathAction::AddCustomIncludePathAction(const KDevelop::IndexedString& url) {
  m_url = url;
}

void Cpp::AddCustomIncludePathAction::execute()
{
    openConfigurationPage(m_url.str());
    emit executed(this);
}

QString Cpp::AddCustomIncludePathAction::description() const {
  return i18n("Add Custom Include Path");
}


Cpp::OpenProjectForFileAssistant::OpenProjectForFileAssistant(const KUrl& url) : m_url(url) {
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

Cpp::MissingIncludePathAssistant::MissingIncludePathAssistant(const KDevelop::IndexedString& url, const QString& directive) {
  m_url = url;
  m_directive = directive;
}

void MissingIncludePathAssistant::createActions()
{
    auto project = KDevelop::ICore::self()->projectController()->findProjectForUrl(m_url.toUrl());

    if (!project) {
        addAction(KDevelop::IAssistantAction::Ptr(new OpenProjectForFileAssistant(m_url.toUrl())));
    }
    addAction(KDevelop::IAssistantAction::Ptr(new AddCustomIncludePathAction(m_url)));
}

KSharedPtr< KDevelop::IAssistant > MissingIncludePathProblem::solutionAssistant() const
{
    return m_solution;
}

void MissingIncludePathProblem::setSolutionAssistant(const KSharedPtr< KDevelop::IAssistant >& assistant)
{
    m_solution = assistant;
}
