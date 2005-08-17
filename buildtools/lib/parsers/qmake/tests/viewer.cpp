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
#include <q3listbox.h>
#include <q3filedialog.h>
#include <q3textedit.h>
#include <qfile.h>
#include <qtextstream.h>
#include <q3listview.h>
#include <qtabwidget.h>

#include <qmakeast.h>
#include <qmakedriver.h>
//Added by qt3to4:
#include <Q3ValueList>

using namespace QMake;

Viewer::Viewer(QWidget *parent, const char *name)
    :ViewerBase(parent, name)
{
    if (QFile::exists(QDir::currentDirPath() + "/" + "qtlist"))
    {
        QFile f(QDir::currentDirPath() + "/" + "qtlist");
        f.open(QIODevice::ReadOnly);
        QTextStream str(&f);
        while (!str.eof())
            files->insertItem(str.readLine());
    }
    ast->setSorting(-1);
    parentProject.push((Q3ListViewItem*)0);
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
    QString fileName = Q3FileDialog::getOpenFileName(QDir::currentDirPath(), "*.pro *.pri", this);
    if (!fileName.isEmpty())
        files->insertItem(fileName);
}

void Viewer::files_currentChanged(Q3ListBoxItem* item)
{
    ast->clear();
    
    QFile f(item->text());
    f.open(QIODevice::ReadOnly);
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
    if ((text == "Source to be written back") && projectAST)
    {
        QString buffer;
        projectAST->writeBack(buffer);
        writeBack->setText(buffer);
    }
}

void Viewer::processAST(QMake::ProjectAST *projectAST, Q3ListViewItem *globAfter)
{
    Q3ListViewItem *projectIt;
    if (!parentProject.top())
        projectIt = new Q3ListViewItem(ast, "Project");
    else
    {
        if (projectAST->isScope())
            projectIt = new Q3ListViewItem(parentProject.top(), globAfter, projectAST->scopedID);
        if (projectAST->isFunctionScope())
            projectIt = new Q3ListViewItem(parentProject.top(), globAfter, 
                projectAST->scopedID + "(" + projectAST->args + ")");
    }
    projectIt->setOpen(true);
    
    Q3ListViewItem *after = 0;
    for (Q3ValueList<QMake::AST*>::const_iterator it = projectAST->statements.constBegin();
            it != projectAST->statements.constEnd(); ++it)
    {
        AST *ast = *it;
        if (ast == 0)
            continue;
        switch (ast->nodeType()) {
            case AST::AssignmentAST: {
                AssignmentAST *assignmentAST = static_cast<QMake::AssignmentAST*>(ast);
                Q3ListViewItem *item = new Q3ListViewItem(projectIt, after,
                        assignmentAST->scopedID, assignmentAST->op, assignmentAST->values.join(""));
                item->setMultiLinesEnabled(true);
                after = item; }
                break;
                
            case AST::NewLineAST:
//                 after = new QListViewItem(projectIt, after, "<newline>");
                break;
                
            case AST::CommentAST:
//                 after = new QListViewItem(projectIt, after, "<comment>");
                break;
                
            case AST::ProjectAST: {
                ProjectAST *projectAST = static_cast<QMake::ProjectAST*>(ast);
                parentProject.push(projectIt);
                processAST(projectAST, after);
                parentProject.pop(); }
                break;
        
            case AST::FunctionCallAST:
                after = new Q3ListViewItem(projectIt, after, "<funccall>");
                break;
        }
    }
}

#include "viewer.moc"
