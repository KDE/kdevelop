/***************************************************************************
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef _SCOPE_H_
#define _SCOPE_H_

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include "qmakeast.h"

#ifdef DEBUG
#include "qmakeastvisitor.h"
#endif

/*
 * TODO:
 * - Ask adymo again about variables, we can't just always put a new assignment at the end of the file, because that clutters the file after a few open/save cycles
        -> recursively check for other assignments to the variable (into scopes)
 * - talk to adymo about the possibility to add a way to parse only part of a QMake project file in case the include-creation code doesn't work
        -> see mail, find out wether the proposed function could be better, flex reading from memory...
 * - talk to adymo why function calls are "automatically" scopes and not function calls
        -> line 243 in parser, in init() do special handling of FunctionCall's, make a FunctionScope+AssignmentAST out of it, or use http://rafb.net/paste/results/EE43DN82.html patch
 * - talk to adymo about memory handling in AST's, do they delete their children upon deletion?
        -> deletion of childs.
 * - talk to adymo about how to handle unparsable file's - overwrite or set m_root to 0?
        -> to rare to handle specially.
 * - Handle multiple function calls with different arguments
 */

class Scope;

class Scope
{
public:

    enum ScopeType {
        ProjectScope,
        FunctionScope,
        SimpleScope,
        IncludeScope,
        InvalidScope
    };
    static const QStringList KnownVariables;
    static const QStringList KnownConfigValues;

    Scope( const QString &filename, bool isQt4Project = false );
    ~Scope();

    void saveToFile() const;

    // Changing variable values
    void addToPlusOp( const QString& variable, const QStringList& values );
    void removeFromPlusOp( const QString& variable, const QStringList& values );
    void addToMinusOp( const QString& variable, const QStringList& values );
    void removeFromMinusOp( const QString& variable, const QStringList& values );
    void addToEqualOp( const QString& variable, const QStringList& values );
    void removeFromEqualOp( const QString& variable, const QStringList& values );
    void setPlusOp( const QString& variable, const QStringList& values );
    void setEqualOp( const QString& variable, const QStringList& values );
    void setMinusOp( const QString& variable, const QStringList& values );

    // Checks wether a line like VAR = exists in this subscope
    bool isVariableReset( const QString& var );

    // Fetch the valuelist for the variable op combination inside this scope
    QStringList variableValuesForOp( const QString& variable, const QString& op ) const;

    // Fetch the variable values by running over the statements and adding/removing/setting
    // as the encountered op's say, begin with the parent projects variableValues list
    QStringList variableValues( const QString& variable ) const;

    // Remove a variable+Op combination from the scope, if existant
    void removeVariable( const QString& var, const QString& op );

    // Getting to know what type of scope this is
    ScopeType scopeType() const;

    // This returns the function+args, the scopename or the pro/pri file
    // depending on the type of scope
    QString scopeName() const;

    // Returns the projectName for this scope, this is equal to the last part of the projectDir()
    QString projectName() const;

    // Returns just the filename of this project's .pro file
    QString fileName() const;

    // Returns the absolute path of the dir containing the .pro file
    QString projectDir() const;

    // get the parent Scope
    Scope* parent() const { return m_parent; }

    // Fetching sub-scopes
    const QValueList<Scope*> functionScopes() const { return m_funcScopes.values(); }
    const QValueList<Scope*> simpleScopes() const { return m_simpleScopes.values(); }
    const QValueList<Scope*> includeScopes() const { return m_incScopes.values(); }
    const QValueList<Scope*> subProjectScopes() const { return m_subProjects.values(); }
    const QValueList<Scope*> scopesInOrder() const;
    // Working on SubScopes
    /*
     * creates a new function scope at the end of this (Sub-)AST and returns the Scope wrapping it
     */
    Scope* createFunctionScope( const QString& funcName, const QString& args );
    /*
     * creates a new simple scope at the end of this (Sub-)AST and returns the Scope wrapping it
     */
    Scope* createSimpleScope( const QString& scopename );

    /*
     * creates a new function scope at the end of this (Sub-)AST
     * and a new include scope inside the new function scope.
     * It returns the Scope wrapping the include-AST, the function scope AST
     * can be accessed easily using the parent() method.
     */
    Scope* createIncludeScope( const QString& includeFile, bool negate = false );

    /*
     * creates a new subproject in dir (create's dir if necessary)
     * If this scope is not a project scope the subproject will be added to this
     * Scope only, i.e. it is not seen in the project-files list of subdirs
     */
    Scope* createSubProject( const QString& dir );

    /* delete the given function scope */
    void deleteFunctionScope( const QString& functionCall );
    /* delete the given simple scope */
    void deleteSimpleScope( const QString& scopeId );
    /* delete the given include scope */
    void deleteIncludeScope( const QString& includeFile, bool negate = false );
    /* deletes the subproject (including the subdir if deleteSubdir is true) */
    void deleteSubProject( const QString& dir, bool deleteSubdir );

    /* find out wether the project is Qt4 or Qt3 */
    bool isQt4Project() const { return m_isQt4Project; }

    /* Provide a Map of Custom variables */
    const QMap<QPair<QString, QString>, QStringList> customVariables() const;

    void removeCustomVariable( const QString& var, const QString& op );

    /* Update the values of the variable/operation combo var+op to values */
    void updateCustomVariable( const QString& var, const QString& op, const QStringList& values );

    // Checks wether a QStringList contains any values that are not whitespace or \\n
    bool listIsEmpty( const QStringList& values );

    bool isEnabled() { return m_isEnabled; }

#ifdef DEBUG
    void printTree();
#endif

private:
    /*
     * Updates the given Variable+op with the values, if removeFromOp is true it removes the values, else it adds them
     * this works it's way back through the current scope and changes the last occurence of op to
     * include all new values.
     *
     * Depending on "op" it might end the search earlier (if op is += it also stops at =)
     *
     * This also removes the values from other assignments if the operation is not op, i.e.
     * if op is += removes values from any occurence of -=
     * if op is -= removes values from any occurence of = and +=
     * if op is = removes values frmo any occurence of -=
     */
    void updateVariable( const QString& variable, const QString& op, const QStringList& values, bool removeFromOp );

    /*
     * Helper Function to change the origValues list with the values from newValues
     * depending on the state of "remove" either adds or removes all entries from newValues
     * to origValues if they didn't exist there yet
     */
    void updateValues( QStringList& origValues, const QStringList& newValues, bool remove );

    /*
     * Finds an existing variable, returns the end() of the statemenst if it is not found
     */
    QValueList<QMake::AST*>::iterator findExistingVariable( const QString& variable );

    // Builds the scope-lists and the customVariables list
    void init();

    // Private constructors for easier subscope creation
    /*
     * just initializes the lists from the scope
     */
    Scope( bool isQt4Project, QMake::ProjectAST* root, Scope* parent );
    /*
     * reads the given filename and parses it. If it doesn't exist creates an empty
     * ProjectAST with the given filename
     */
    Scope( bool isQt4Project, Scope* parent, const QString& filename );
    Scope( bool isQt4Project, Scope* parent, const QString& filename, bool isEnabled );
    /*
     * Creates a scope for an include statement, parses the file and initializes the Scope
     * Create an empty ProjectAST if the file cannot be found or parsed.
     */
    Scope( bool isQt4Project, Scope* parent, QMake::IncludeAST* incast, const QString& path, const QString& incfile );


    // runs through the statements until stopHere is found (or the end is reached, if stopHere is 0),
    // using the given list as startvalue
    // Changes the list using the +=, -=, = operations accordingly
    QStringList calcValuesFromStatements( const QString& variable, QStringList result, QMake::AST* stopHere = 0 ) const;

    // Check wether the two operators are compatible
    bool isCompatible( const QString& op1, const QString& op2) const;

    // Check wether the 2 lists are equal, regardless of element order.
    bool listsEqual(const QStringList& , const QStringList& ) const;

    // Load and Save project files, these only work on ProjectScope's
    bool loadFromFile( const QString& filename );

    QString funcScopeKey( QMake::ProjectAST* funcast ) const { return funcast->scopedID + "(" + funcast->args + ")"; }

    QMake::ProjectAST* m_root;
    QMake::IncludeAST* m_incast;
    QMap<QPair<QString, QString>, QMake::AssignmentAST*> m_customVariables;
    Scope* m_parent;

    // subProjects and includes are separated because their keys could interfere with simple scope keys
    QMap<QString, Scope*> m_simpleScopes;
    QMap<QString, Scope*> m_funcScopes;
    QMap<QString, Scope*> m_incScopes;
    QMap<QString, Scope*> m_subProjects;
    bool m_isQt4Project;

    /* Caching is a bit complicated, need to store var+op as key and inside the calcValuesFromStatements
     * need to fetch values from parents, then iterate over the 3 ops and accordingly update the list, then
     * we can return the result and we save the iteration of our own statement list
     */
    QMap<QString, QStringList> m_varCache;
    bool m_isEnabled;
#ifdef DEBUG
    class PrintAST : QMake::ASTVisitor
    {

    public:
        PrintAST();
        virtual void processProject( QMake::ProjectAST* p );
        virtual void enterRealProject( QMake::ProjectAST* p );

        virtual void leaveRealProject( QMake::ProjectAST* p );

        virtual void enterScope( QMake::ProjectAST* p );

        virtual void leaveScope( QMake::ProjectAST* p );

        virtual void enterFunctionScope( QMake::ProjectAST* p );

        virtual void leaveFunctionScope( QMake::ProjectAST* p );

        virtual void processAssignment( QMake::AssignmentAST* a);

        virtual void processNewLine( QMake::NewLineAST* n);

        virtual void processComment( QMake::CommentAST* a);

        virtual void processInclude( QMake::IncludeAST* a);

    private:
        QString getIndent();
        QString replaceWs(QString);
        int indent;

    };
#endif

};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

