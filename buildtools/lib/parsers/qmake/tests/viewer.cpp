/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "viewer.h"

#include <qdir.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qfiledialog.h>
#include <qtextedit.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlistview.h>
#include <qtabwidget.h>

#include <qmakeast.h>
#include <qmakedriver.h>
#include <qmakeastvisitor.h>

using namespace QMake;

Viewer::Viewer(QWidget *parent, const char *name)
    :ViewerBase(parent, name), projectAST(0)
{
    if (QFile::exists(QDir::currentDirPath() + "/" + "qtlist"))
    {
        QFile f(QDir::currentDirPath() + "/" + "qtlist");
        f.open(IO_ReadOnly);
        QTextStream str(&f);
        while (!str.eof())
            files->insertItem(str.readLine());
    }
    ast->setSorting(-1);
//    parentProject.push((QListViewItem*)0);
}

void Viewer::addAll_clicked()
{
    if (allLocation->text().isEmpty())
        return;
    QDir d(allLocation->text());
    QStringList l = d.entryList("*.pro *.pri");
    for (QStringList::iterator it = l.begin(); it != l.end(); ++it)
        (*it) = QDir::cleanDirPath(allLocation->text() + "/" + (*it));
    files->insertStringList(l);
}

void Viewer::choose_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(QDir::currentDirPath(), "*.pro *.pri", this);
    if (!fileName.isEmpty())
        files->insertItem(fileName);
}

void Viewer::files_currentChanged(QListBoxItem* item)
{
    ast->clear();

    QFile f(item->text());
    f.open(IO_ReadOnly);
    QTextStream str(&f);
    source->setText(str.read());
    f.close();

    int result = QMake::Driver::parseFile(item->text().ascii(), &projectAST);
    if (projectAST && (result == 0))
    {
        processAST(projectAST);
    }
    if (tabWidget2->currentPageIndex() == 1)
        tabWidget2_selected("Source to be written back");
}

void Viewer::tabWidget2_selected(const QString& text)
{
    if ((text == "Source to Be Written Back") && projectAST)
    {
        QString buffer;
        projectAST->writeBack(buffer);
        writeBack->setText(buffer);
    }
}

class ViewerVisitor: public ASTVisitor {
public:
    ViewerVisitor(Viewer *v): ASTVisitor()
    {
        this->v = v;
        parentProject.push((QListViewItem*)0);
    }

    virtual void processProject(ProjectAST *project)
    {
        ASTVisitor::processProject(project);
    }

    virtual void enterRealProject(ProjectAST *project)
    {
        QListViewItem *projectIt;
        if (!parentProject.top())
        {
            projectIt = new QListViewItem(v->ast, "Project");
            projectIt->setOpen(true);
            parentProject.push(projectIt);
        }

        ASTVisitor::enterRealProject(project);
    }
    virtual void enterScope(ProjectAST *scope)
    {
        QListViewItem *projectIt = new QListViewItem(parentProject.top(), scope->scopedID, "scope");
        parentProject.push(projectIt);
        ASTVisitor::enterScope(scope);
    }
    virtual void leaveScope(ProjectAST *scope)
    {
        parentProject.pop();
    }
    virtual void enterFunctionScope(ProjectAST *fscope)
    {
        QListViewItem *projectIt = new QListViewItem(parentProject.top(),
            fscope->scopedID + "(" + fscope->args + ")", "function scope");
        parentProject.push(projectIt);
        ASTVisitor::enterFunctionScope(fscope);
    }
    virtual void leaveFunctionScope(ProjectAST *fscope)
    {
        parentProject.pop();
    }
    virtual void processAssignment(AssignmentAST *assignment)
    {
        QListViewItem *item = new QListViewItem(parentProject.top(),
                assignment->scopedID, assignment->op, assignment->values.join("|"),
                "assignment");
        item->setMultiLinesEnabled(true);

        ASTVisitor::processAssignment(assignment);
    }
    virtual void processNewLine(NewLineAST *newline)
    {
        new QListViewItem(parentProject.top(), "<newline>");
        ASTVisitor::processNewLine(newline);
    }
    virtual void processComment(CommentAST *comment)
    {
        new QListViewItem(parentProject.top(), "<comment>");
        ASTVisitor::processComment(comment);
    }
    virtual void processInclude(IncludeAST *include)
    {
        new QListViewItem(parentProject.top(), "<include>", include->projectName);
        QMake::ASTVisitor::processInclude(include);
    }

    Viewer *v;
    QValueStack<QListViewItem *> parentProject;
};


void Viewer::processAST(QMake::ProjectAST *projectAST, QListViewItem *globAfter)
{
    ViewerVisitor visitor(this);
    visitor.processProject(projectAST);
}

#include "viewer.moc"
