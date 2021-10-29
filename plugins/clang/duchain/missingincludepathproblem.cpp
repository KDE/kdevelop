/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    explicit AddCustomIncludePathAction(const KDevelop::IndexedString& path)
        : m_path(path)
    {}

    QString description() const override
    {
        return i18n("Add Custom Include Path");
    }

    void execute() override
    {
        openConfigurationPage(m_path.str());
        emit executed(this);
    }

private:
    KDevelop::IndexedString m_path;
};

class OpenProjectForFileAssistant : public KDevelop::IAssistantAction
{
    Q_OBJECT
public:
    explicit OpenProjectForFileAssistant(const KDevelop::IndexedString& path)
        : m_path(path)
    {}

    QString description() const override
    {
        return i18n("Open Project");
    };

    void execute() override
    {
        KDevelop::ICore::self()->projectController()->openProjectForUrl(m_path.toUrl());
        emit executed(this);
    }

private:
    KDevelop::IndexedString m_path;
};

class MissingIncludePathAssistant : public ClangFixitAssistant
{
    Q_OBJECT
public:
    MissingIncludePathAssistant(const QString& title, const KDevelop::IndexedString& path)
        : ClangFixitAssistant(title, {})
        , m_path(path)
    {}

    void createActions() override
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

MissingIncludePathProblem::MissingIncludePathProblem(CXDiagnostic diagnostic, CXTranslationUnit unit)
    : ClangProblem(diagnostic, unit)
{}

KDevelop::IAssistant::Ptr MissingIncludePathProblem::solutionAssistant() const
{
    return KDevelop::IAssistant::Ptr(new MissingIncludePathAssistant(description(), finalLocation().document));
}

#include "missingincludepathproblem.moc"
