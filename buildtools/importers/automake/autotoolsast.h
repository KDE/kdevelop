/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   Copyright (c) 2005 by Matt Rogers                                     *
 *   mattr@kde.org                                                         *
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
#ifndef AUTOTOOLSAST_H
#define AUTOTOOLSAST_H

#include <QList>
#include <QStringList>

/**
@file autotools.h
Abstract Syntax Tree (AST) class declarations.
*/

namespace AutoTools
{

/**
 * AST node.
 * This is the base class. Objects of this type are not created by the parser.
 *
 * Each AST node holds the list of its children which are always deleted in the
 * destructor. This way, it's possible call delete for only root AST node and
 * others will be deleted automatically.
 *
 * Each AST node also knows how to write the information back into .pro file.
 */
class AST
{
public:
	/**Type of AST node.*/
	enum NodeType {
		ProjectAST,                 ///< Project, scope or function scope.
		AssignmentAST,              ///< Variable assignment.
		TargetAST,                  ///< Automake target
		MakefileConditionalAST,     ///< Makefile.am conditional 
		NewLineAST,                 ///< Line feed.
		CommentAST                  ///< Comment.
	};
	
	/** Constructs AST with given node type.*/
	AST(NodeType nodeType): m_nodeType(nodeType), m_depth(0) {}
	virtual ~AST();
	
	/**
	 * Adds child AST node to this node. Despite this function is virtual,
	 * reimplementations should call it to make automatic destruction of
	 * AST tree possible.*/
	virtual void addChildAST(AST *node);
	
	/** 
	 * Writes information stored in the AST into the @p buffer.
	 * This is a default implementation which iterates over child nodes
	 * and calls writeBack for each child node.
	 */
	virtual void writeBack(QString &buffer);
	
	/** @return The type of the node.*/
	virtual NodeType nodeType() const { return m_nodeType; }
	
	/** Sets the depth of the node in AST.*/
	void setDepth(int depth) { m_depth = depth; }
	
	/** @return The depth of the node in AST.*/
	int depth() const { return m_depth; }
	
	/** @return The indentation string based on node depth.*/
	virtual QString indentation();
	
	//! \return true if this AST has children
	bool hasChildren() const;

	/**
	 * Get the children of this ast
	 * \return the list of this ast's childre
	 */
	QList<AST*> children() const;


	
protected:
	NodeType m_nodeType;
	QList<AST*> m_children;
	
private:
	int m_depth;

};


/**
 * Project AST node.
 * Represents complete projects, scopes and function scopes.
 * Examples:
 * @code
 * scopename{
 * var=value
 * }
 * function(args){
 * var=value
 * }
 * @endcode
 */
class ProjectAST: public AST
{
public:
	/**The kind of a project node.*/
	enum Kind
	{
		Project,            ///< Project
		Target,             ///< Custom Automake Target
		ConditionalScope,   ///< Automake Conditional
		Rule,               ///< Automake Rule
		Empty               ///< Project does not exist. the AST is empty
	};
	
	/** Constructs a project node of given @p kind. */
	ProjectAST(Kind kind = Project): AST(AST::ProjectAST), m_kind(kind) {}
	
	virtual void writeBack(QString &buffer);
	virtual void addChildAST(AST *node);
	
	/** @return true if this node is a project.*/
	bool isProject() const { return m_kind == Project; }
	
	bool isRule() const { return m_kind == Rule; }
	
	/** @return true if this node is an automake conditional */
	bool isConditionalScope() const { return m_kind == ConditionalScope; }
	
	/** @return true if this node is empty.*/
	bool isEmpty() const { return m_kind == Empty; }
	
	/**Scoped identifier (scope name or rule).*/
	QString scopedID;
	
	/**Function arguments. Empty for other kinds of projects.*/
	QString args;
	
	/** The automake conditional has an else attached */
	bool hasElse;
	
	/**List of statements.*/
	QList<AutoTools::AST*> statements;
	
private:
	Kind m_kind;

};


/**
 * Assignment AST node.
 * Represents assignments, for example:
 * \code
 * var=value
 * \endcode
 *
 * Values can be specified on several lines and
 * each line is stored as a string in the list of values.@n
 * For example, if we have in .pro:
 * \code
 * SOURCES=a.cpp \
 * b.cpp c.cpp
 * \endcode 
 * then values will be stored as a two elements list:
 * \code
 * a.cpp
 * b.cpp c.cpp
 * \endcode
 */
class AssignmentAST: public AST
{
public:
	AssignmentAST(): AST(AST::AssignmentAST) {}

	virtual void writeBack(QString &buffer);
	
	/**Scoped name of the variable.*/
	QString scopedID;
	
	/**Operator.*/
	QString op;
	
	/**List of values.*/
	QStringList values;
};

class AutomakeTargetAST : public AST
{
public:
	AutomakeTargetAST() : AST(AST::TargetAST) {}
	
	virtual void writeBack( QString& buffer );
	
	/// The name of the target
	QString target;
	
	/// The dependencies for the target, if any
	QStringList deps;
};

class ConditionAST : public AST
{
public:
	ConditionAST() : AST( AST::MakefileConditionalAST ) {}
	
	virtual void writeBack( QString& buffer );
	
	/// The keyword for the condition (if, else, endif)
	QString type;
		
	/// The name of the condition
	QString conditionName;
};

/**
 * New line AST node.
 * Represents line feeds in files.
 */
class NewLineAST: public AST
{
public:
	NewLineAST(): AST(AST::NewLineAST) {}
	
	virtual void writeBack(QString &buffer);
};


/**
 * Comment AST node.
 * Represents comments.
 */
class CommentAST: public AST
{
public:
	CommentAST(): AST(AST::CommentAST) {}
	
	virtual void writeBack(QString &buffer);
	
	/**Comment text.*/
	QString comment;
	
};

}

#endif

// kate: indent-mode csands; space-indent off; tab-width 4; auto-insert-doxygen on;
