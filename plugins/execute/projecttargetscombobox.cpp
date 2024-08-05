/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projecttargetscombobox.h"

#include <QIcon>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <util/kdevstringhandler.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

using namespace KDevelop;

ProjectTargetsComboBox::ProjectTargetsComboBox(QWidget* parent)
    : QComboBox(parent)
{

}
    
class ExecutablePathsVisitor
    : public ProjectVisitor
{
    public:
        explicit ExecutablePathsVisitor(bool exec) : m_onlyExecutables(exec) {}
        using ProjectVisitor::visit;
        void visit(ProjectExecutableTargetItem* eit) override {
            if(!m_onlyExecutables || eit->type()==ProjectTargetItem::ExecutableTarget)
                m_paths += KDevelop::joinWithEscaping(eit->model()->pathFromIndex(eit->index()), QLatin1Char('/'), QLatin1Char('\\'));
        }

        QStringList paths() const { return m_paths; }

        void sort() { m_paths.sort(); }

    private:
        bool m_onlyExecutables;
        QStringList m_paths;
};


void ProjectTargetsComboBox::setBaseItem(ProjectFolderItem* item, bool exec)
{
    clear();

    QList<ProjectFolderItem*> items;
    if(item) {
        items += item;
    } else {
        const auto projects = ICore::self()->projectController()->projects();
        items.reserve(projects.size());
        for (auto* p : projects) {
            items += p->projectItem();
        }
    }

    ExecutablePathsVisitor walker(exec);
    for (ProjectFolderItem* item : std::as_const(items)) {
        walker.visit(item);
    }
    walker.sort();

    const auto executableItems = walker.paths();
    for (const QString& item : executableItems) {
        addItem(QIcon::fromTheme(QStringLiteral("system-run")), item);
    }

}

QStringList ProjectTargetsComboBox::currentItemPath() const
{
    return KDevelop::splitWithEscaping(currentText(), QLatin1Char('/'), QLatin1Char('\\'));
}

void ProjectTargetsComboBox::setCurrentItemPath(const QStringList& str)
{
    setCurrentIndex(str.isEmpty() && count() ? 0 : findText(KDevelop::joinWithEscaping(str, QLatin1Char('/'), QLatin1Char('\\'))));
}

#include "moc_projecttargetscombobox.cpp"
