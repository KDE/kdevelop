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
        ExecutablePathsVisitor(bool exec) : m_onlyExecutables(exec) {}
        using ProjectVisitor::visit;
        virtual void visit(ProjectExecutableTargetItem* eit) override {
            if(!m_onlyExecutables || eit->type()==ProjectTargetItem::ExecutableTarget)
                m_paths += KDevelop::joinWithEscaping(eit->model()->pathFromIndex(eit->index()), '/', '\\');
        }

        QStringList paths() const { return m_paths; }

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
        foreach(IProject* p, ICore::self()->projectController()->projects()) {
            items += p->projectItem();
        }
    }
    
    ExecutablePathsVisitor walker(exec);
    foreach(ProjectFolderItem* item, items) {
        walker.visit(item);
    }
    
    foreach(const QString& item, walker.paths())
        addItem(QIcon::fromTheme("system-run"), item);
    
}

QStringList ProjectTargetsComboBox::currentItemPath() const
{
    return KDevelop::splitWithEscaping(currentText(), '/', '\\');
}

void ProjectTargetsComboBox::setCurrentItemPath(const QStringList& str)
{
    setCurrentIndex(str.isEmpty() && count() ? 0 : findText(KDevelop::joinWithEscaping(str, '/', '\\')));
}
