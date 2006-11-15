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
#include <qmakeast.h>
#include <qmakeastvisitor.h>
#include <qmakedriver.h>
#include <kdebug.h>

class PrintAST : QMake::ASTVisitor
{
public:
  PrintAST() : QMake::ASTVisitor()
  {
      indent = 0;
  }

  virtual void processProject( QMake::ProjectAST* p )
  {
      QMake::ASTVisitor::processProject(p);
  }
private:
  virtual void enterRealProject( QMake::ProjectAST* p )
  {
      kdDebug(9024) << getIndent() << "--------- Entering Project: " << replaceWs(p->fileName()) << " --------------" << endl;
      indent += 4;
      QMake::ASTVisitor::enterRealProject(p);
  }

  virtual void leaveRealProject( QMake::ProjectAST* p )
  {
      indent -= 4;
      kdDebug(9024) << getIndent() << "--------- Leaving Project: " << replaceWs(p->fileName()) << " --------------" << endl;
      QMake::ASTVisitor::leaveRealProject(p);
  }

  virtual void enterScope( QMake::ProjectAST* p )
  {
      kdDebug(9024) << getIndent() << "--------- Entering Scope: " << replaceWs(p->scopedID) << " --------------" << endl;
      indent += 4;
      QMake::ASTVisitor::enterScope(p);
  }

  virtual void leaveScope( QMake::ProjectAST* p )
  {
      indent -= 4;
      kdDebug(9024) << getIndent() << "--------- Leaving Scope: " << replaceWs(p->scopedID) << " --------------" << endl;
      QMake::ASTVisitor::leaveScope(p);
  }

  virtual void enterFunctionScope( QMake::ProjectAST* p )
  {
      kdDebug(9024) << getIndent() << "--------- Entering FunctionScope: " << replaceWs(p->scopedID) << "(" << replaceWs(p->args) << ")"<< " --------------" << endl;
      indent += 4;
      QMake::ASTVisitor::enterFunctionScope(p);
  }

  virtual void leaveFunctionScope( QMake::ProjectAST* p )
  {
      indent -= 4;
      kdDebug(9024) << getIndent() << "--------- Leaving FunctionScope: " << replaceWs(p->scopedID) << "(" << replaceWs(p->args) << ")"<< " --------------" << endl;
      QMake::ASTVisitor::leaveFunctionScope(p);
  }

  QString replaceWs(QString s)
  {
      return s.replace("\n", "%nl").replace("\t", "%tab").replace(" ", "%spc");
  }

  virtual void processAssignment( QMake::AssignmentAST* a)
  {
      if( a->commentnode != 0 )
	      kdDebug(9024) << getIndent() << "Assignment(" << replaceWs(a->indent) << "):" << replaceWs(a->scopedID) << " " << replaceWs(a->op) << " " << replaceWs(a->values.join("|")) << static_cast<QMake::CommentAST*>(a->commentnode)->comment << endl;
      else
	      kdDebug(9024) << getIndent() << "Assignment(" << replaceWs(a->indent) << "):" << replaceWs(a->scopedID) << " " << replaceWs(a->op) << " " << replaceWs(a->values.join("|")) << endl;
      QMake::ASTVisitor::processAssignment(a);
  }

  virtual void processNewLine( QMake::NewLineAST* n)
  {
      kdDebug(9024) << getIndent() << "Newline " << endl;
      QMake::ASTVisitor::processNewLine(n);
  }

  virtual void processComment( QMake::CommentAST* a)
  {
      kdDebug(9024) << getIndent() << "Comment: " << replaceWs(a->comment) << endl;
      QMake::ASTVisitor::processComment(a);
  }

  virtual void processInclude( QMake::IncludeAST* a)
  {
      kdDebug(9024) << getIndent() << "Include: " << replaceWs(a->projectName) << endl;
      QMake::ASTVisitor::processInclude(a);
  }

  QString getIndent()
  {
      QString ind;
      for( int i = 0 ; i < indent ; i++)
          ind += " ";
      return ind;
  }
  int indent;
};
int main(int argc, char *argv[])
{
    QMake::ProjectAST *projectAST;
    int ret;
    if (argc > 1)
    {
        ret = QMake::Driver::parseFile(argv[1], &projectAST);
        PrintAST pa;
        if ( ret == 0 )
            if ((argc < 3) || ((argc == 3) && (strcmp(argv[2], "--silent") != 0)))
            {
                pa.processProject(projectAST);
                QString profile;
                projectAST->writeBack(profile);
                kdDebug(9024) << "QMake file written back:\n" << profile << endl;
            }
        return ret;
    }
    else
        return 0;
}
