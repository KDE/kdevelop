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
#include <qobject.h>

#include "qmakeast.h"
#include "qmakedefaultopts.h"

#ifdef DEBUG
#include "qmakeastvisitor.h"
#endif

class Scope;
class TrollProjectPart;

class Scope : public QObject
{
    Q_OBJECT
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

    Scope( const QString &filename, TrollProjectPart* part );
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
    QStringList variableValues( const QString& variable, bool checkIncParent = true );

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
//     const QValueList<Scope*> functionScopes() const;
//     const QValueList<Scope*> simpleScopes() const;
//     const QValueList<Scope*> includeScopes() const;
//     const QValueList<Scope*> subProjectScopes() const;
    const QValueList<Scope*> scopesInOrder() const { return m_scopes.values(); }
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
    bool deleteFunctionScope( unsigned int );
    /* delete the given simple scope */
    bool deleteSimpleScope( unsigned int );
    /* delete the given include scope */
    bool deleteIncludeScope( unsigned int );
    /* deletes the subproject (including the subdir if deleteSubdir is true) */
    bool deleteSubProject( unsigned int, bool deleteSubdir );

    /* find out wether the project is Qt4 or Qt3 */
    bool isQt4Project() const ;

    /* Provide a Map of Custom variables */
    const QMap<unsigned int, QMap<QString, QString> > customVariables() const;

    unsigned int addCustomVariable( const QString& var, const QString& op, const QString& values );

    /* Removes the variable with the given id if it exists */
    void removeCustomVariable( unsigned int );

    /* Update the values of the variable/operation combo var+op to values */
    void updateCustomVariable( unsigned int, const QString&, const QString& , const QString& );

    // Checks wether a QStringList contains any values that are not whitespace or \\n
    static bool listIsEmpty( const QStringList& values );

    /* returns wether this is an enabled subproject or a disabled one */
    bool isEnabled() { return m_isEnabled; }

    static QStringList removeWhiteSpace(const QStringList& list);

    /* Reload a project scope */
    void reloadProject();

    /* creates a new disabled Scope child and add SUBDIRS -= dir to this scope */
    Scope* disableSubproject( const QString& );

    /* return the "position" of this scope in the list of scopes */
    unsigned int getNum() { return m_num; }

    QStringList allFiles( const QString& );

    bool isInitializationFinished() const { return m_initFinished; }

#ifdef DEBUG
    void printTree();
#endif

signals:
    void initializationFinished();

private slots:
    // Builds the scope-lists and the customVariables list
    void init();

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
    void updateValues( QStringList& origValues, const QStringList& newValues, bool remove = false, QString indent = "  " );

    /*
     * Finds an existing variable, returns the end() of the statemenst if it is not found
     */
    QValueList<QMake::AST*>::iterator findExistingVariable( const QString& variable );

    // Private constructors for easier subscope creation
    /*
     * just initializes the lists from the scope
     */
    Scope( unsigned int num, Scope* parent, QMake::ProjectAST* root, QMakeDefaultOpts*, TrollProjectPart* part );
    /*
     * reads the given filename and parses it. If it doesn't exist creates an empty
     * ProjectAST with the given filename
     */
    Scope( unsigned int num, Scope* parent, const QString& filename, TrollProjectPart* part, QMakeDefaultOpts*, bool isEnabled = true );
    /*
     * Creates a scope for an include statement, parses the file and initializes the Scope
     * Create an empty ProjectAST if the file cannot be found or parsed.
     */
    Scope( unsigned int num, Scope* parent, QMake::IncludeAST* incast, const QString& path, const QString& incfile, QMakeDefaultOpts*, TrollProjectPart* part );


    // runs through the statements until stopHere is found (or the end is reached, if stopHere is 0),
    // using the given list as startvalue
    // Changes the list using the +=, -=, = operations accordingly
    void calcValuesFromStatements( const QString& variable, QStringList& result, bool, QMake::AST* stopHere = 0 ) const;

    // Check wether the two operators are compatible
    static bool isCompatible( const QString& op1, const QString& op2);

    // Check wether the 2 lists are equal, regardless of element order.
    static bool listsEqual(const QStringList& , const QStringList& );

    // Load and Save project files, these only work on ProjectScope's
    bool loadFromFile( const QString& filename );

    QString funcScopeKey( QMake::ProjectAST* funcast ) const { return funcast->scopedID + "(" + funcast->args + ")"; }

    unsigned int getNextScopeNum() { if( m_scopes.isEmpty() ) return 0; else return (m_scopes.keys().last()+1); }

    QStringList lookupVariable( const QString& var );

    QStringList resolveVariables( const QStringList&, QMake::AST* = 0 ) const;
    QStringList variableValues( const QString& variable, QMake::AST* ) const;
    QString resolveVariables( const QString& , QMake::AST* = 0 ) const;

    void allFiles( const QString&, QStringList& );

    QMake::ProjectAST* m_root;
    QMake::IncludeAST* m_incast;
    QMap<unsigned int, QMake::AssignmentAST*> m_customVariables;
    QMap<unsigned int, Scope*> m_scopes;
    Scope* m_parent;
    unsigned int m_maxCustomVarNum;

    // All different subscopes of this scope, the key is the "position" at which the scope starts
    QMap<QString, Scope*> m_subProjects;

    // The "position" inside the parent scope that this scope starts at
    unsigned int m_num;
    bool m_isEnabled;
    TrollProjectPart* m_part;
    QMakeDefaultOpts* m_defaultopts;
    bool m_initFinished;
    QMap<QString, QStringList> m_varCache;

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

