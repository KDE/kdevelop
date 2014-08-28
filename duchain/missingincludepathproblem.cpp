/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "missingincludepathproblem.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <custom-definesandincludes/idefinesandincludesmanager.h>

#include <KLocalizedString>

namespace
{
void openConfigurationPage(const QString& path)
{
    KDevelop::IDefinesAndIncludesManager::manager()->openConfigurationDialog(path);
}
}

class AddCustomIncludePathAction : public KDevelop::IAssistantAction
{
    Q_OBJECT

public:
    AddCustomIncludePathAction(const KDevelop::IndexedString& path)
        : m_path(path)
    {}

    virtual QString description() const override
    {
        return i18n("Add Custom Include Path");
    }

    virtual void execute() override
    {
        openConfigurationPage(m_path.str());
        emit executed(this);
    }

private:
    KDevelop::IndexedString m_path;
};

class OpenProjectForFileAssistant : public KDevelop::IAssistantAction
{
public:
    OpenProjectForFileAssistant(const KDevelop::IndexedString& path)
        : m_path(path)
    {}

    virtual QString description() const override
    {
        return i18n("Open Project");
    };

    virtual void execute() override
    {
        KDevelop::ICore::self()->projectController()->openProjectForUrl(m_path.toUrl());
        emit executed(this);
    }

private:
    KDevelop::IndexedString m_path;
};

class MissingIncludePathAssistant : public ClangFixitAssistant
{
public:
    MissingIncludePathAssistant(const QString& title, const KDevelop::IndexedString& path)
        : ClangFixitAssistant(title, {})
        , m_path(path)
    {}

    virtual void createActions() override
    {
        auto project = KDevelop::ICore::self()->projectController()->findProjectForUrl(m_path.toUrl());

        if (!project) {
            addAction(KDevelop::IAssistantAction::Ptr(new OpenProjectForFileAssistant(m_path)));
        }
        addAction(KDevelop::IAssistantAction::Ptr(new AddCustomIncludePathAction(m_path)));
    }

private:
    KDevelop::IndexedString m_path;
};

MissingIncludePathProblem::MissingIncludePathProblem(CXDiagnostic diagnostic)
    : ClangProblem(diagnostic)
{}

KDevelop::IAssistant::Ptr MissingIncludePathProblem::solutionAssistant() const
{
    return KDevelop::IAssistant::Ptr(new MissingIncludePathAssistant(description(), finalLocation().document));
}

#include "missingincludepathproblem.moc"
