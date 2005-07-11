/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "qmakeprojectmodel.h"

#include <qmakeast.h>
#include <kdevprojectmodel.h>

using namespace QMake;
//QMakeFolderModel

QMakeFolderModel::QMakeFolderModel(ProjectModel* projectModel)
    :ProjectFolderModel(projectModel), ast(0)
{
}

QMakeFolderModel::~QMakeFolderModel()
{
//     delete ast;
}


//QMakeFileModel

QMakeFileModel::QMakeFileModel(ProjectModel *projectModel)
    :ProjectFileModel(projectModel)
{
}


//QMakeTargetModel

QMakeTargetModel::QMakeTargetModel(ProjectModel *projectModel)
    :ProjectTargetModel(projectModel), ast(0)
{
}

QMakeTargetModel::~QMakeTargetModel()
{
//     delete ast;
}

QStringList QMakeFolderModel::readAssignment( const QString& scopeid, QString &mode ) const
{
	QStringList values;
	if( ast == 0)
		return values;
	QValueList<QMake::AST*>::iterator it = ast->statements.begin();
	for(; it != ast->statements.end(); ++it)
	{
		if( (*it)->nodeType() == AST::AssignmentAST )
		{
			QMake::AssignmentAST *assn = (QMake::AssignmentAST*)(*it);
			if( assn->scopedID == scopeid)
			{
				mode = assn->op;
				QStringList::ConstIterator item = assn->values.begin();
				for(; item != assn->values.end(); ++item)
				{
					QString splitString = (*item).simplifyWhiteSpace();
					QStringList splitItems = QStringList::split(' ', splitString);
					QStringList::ConstIterator subItem = splitItems.begin();
					for(; subItem != splitItems.end(); ++subItem)
					{
						QString subItemText = (*subItem).simplifyWhiteSpace();
						if( !subItemText.isEmpty() && subItemText != "\\")
						{
							values += subItemText;
						}
					}
				}
			}
		}
	}
	return values;
}

QStringList QMakeFolderModel::config() const
{
	QStringList confvars;
	QValueList<QMake::AST*>::iterator it = ast->statements.begin();
	for(; it != ast->statements.end(); ++it)
	{
		if( (*it)->nodeType() == AST::AssignmentAST )
		{
			QMake::AssignmentAST *assn = (QMake::AssignmentAST*)(*it);
			if( assn->scopedID == "CONFIG")
			{
				QStringList::ConstIterator item = assn->values.begin();
				for(; item != assn->values.end(); ++item)
				{
					QString splitString = (*item).simplifyWhiteSpace();
					QStringList splitItems = QStringList::split(' ', splitString);
					QStringList::ConstIterator subItem = splitItems.begin();
					for(; subItem != splitItems.end(); ++subItem)
					{
						QString subItemText = (*subItem).simplifyWhiteSpace();
						if( !subItemText.isEmpty() && subItemText != "\\")
						{
							confvars += subItemText;
						}
					}
				}
			}
		}
	}
	return confvars;
}

void QMakeFolderModel::setConfig( const QStringList &conf )
{
	QMake::AssignmentAST *assn = 0;
	QValueList<QMake::AST*>::iterator it = ast->statements.begin();
	for(; it != ast->statements.end(); ++it)
	{
		if( (*it)->nodeType() == AST::AssignmentAST )
		{
			assn = (QMake::AssignmentAST*)(*it);
			if( assn->scopedID == "CONFIG")
			{
				break;
			}
		}
	}

	if( conf.isEmpty() && assn )
	{
		ast->statements.remove(assn);
		delete assn;
		return;
	}
	
	if( assn == 0 )
	{
		assn = new QMake::AssignmentAST();
		assn->scopedID = "CONFIG";
		assn->op = "+=";
		ast->statements += assn;
	}
	
	assn->values = conf;

}

void QMakeFolderModel::writeScopeID( const QString &scopeid, const QString &mode, const QStringList values )
{	
	QMake::AssignmentAST *assn = 0;
	QValueList<QMake::AST*>::iterator it = ast->statements.begin();
	for(; it != ast->statements.end(); ++it)
	{
		if( (*it)->nodeType() == AST::AssignmentAST )
		{
			assn = (QMake::AssignmentAST*)(*it);
			if( assn->scopedID == scopeid)
			{
				break;
			}
		}
	}
	
	if( values.isEmpty() && assn )
	{
		ast->statements.remove(assn);
		delete assn;
		return;
    }

    if( assn == 0 )
    {
        assn = new QMake::AssignmentAST();
        assn->scopedID = scopeid;
        assn->op = mode;
        ast->statements += assn;
    }

    QStringList::ConstIterator value = values.begin();
    for(; value != values.end(); ++value)
            assn->values += (*value) + " \\";
}
QStringList  QMakeFolderModel::assignmentNames( ) const
{
    QStringList returnList;
    QMake::AssignmentAST *assn = 0;
    QValueList<QMake::AST*>::iterator it = ast->statements.begin();
    for(; it != ast->statements.end(); ++it)
    {
        if( (*it)->nodeType() == AST::AssignmentAST )
        {
            assn = (QMake::AssignmentAST*)(*it);
            returnList += assn->scopedID;
        }
    }
    return returnList;
}
//kate: space-indent on; indent-width 4; mixedindent off;

