/*
  KDevelop Autotools Support
  Copyright (c) 2005 by Matt Rogers <mattr@kde.org>

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
*/
#ifndef MAKEFILEHANDLER_H
#define MAKEFILEHANDLER_H

/**
 * This class is responsible for taking the data from the Makefile.am parser
 * and providing various data list view items can use in the automake manager
 * widget
 * @author Matt Rogers
 */
namespace AutoTools {
class ProjectAST;
}

class MakefileHandler
{
public:
    MakefileHandler();
    ~MakefileHandler();

    /**
     * Parse a folder that has supported makefiles in it. The following files
     * will be looked for in the following order:
     * \li Makefile.am.in
     * \li Makefile.am
     * \li Makefile.in
     *
     * \param folder, the folder to parse
     * \param recursive if true, subfolders will be parsed - defaults to true
     */
    void parse( const QString& folder, bool recursive = true );

    /**
     * Get the AST for a certain path
     * \param folderPath the path of the folder to get an AST for
     * \return the AST that represents a particular folder
     */
    AutoTools::ProjectAST* astForFolder( const QString& folderPath );

    /**
     * Check if a string is an automake variable
     * \return true if it is an automake variable, false otherwise
     */
    bool isVariable( const QString& item ) const;

    /**
     * Find the value for the variable specified by \p variable
     * \param variable The name of the variable to look for
     * \param ast the AST to use to look for the variable in
     * \return the value to substitute for the variable
     */
    QString resolveVariable( const QString& variable,
                             AutoTools::ProjectAST* ast );


private:
    class Private;
    Private* d;
};

#endif

//kate: space-indent on; indent-width 4;

