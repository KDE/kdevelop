/*  This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
    for (ProjectFolderItem* item : qAsConst(items)) {
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
