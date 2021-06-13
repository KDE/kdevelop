/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PROJECTSGENERATOR_H
#define PROJECTSGENERATOR_H

#include "testsexport.h"

namespace KDevelop
{
class IProject;


/// Simple class for generating projects at runtime for testing purposes.
class KDEVPLATFORMTESTS_EXPORT ProjectsGenerator
{
public:
    /**
     * Generates a pointer to a project with the following directory structure:
     * ./simpleproject.kdev4
     * ./src/main.cpp
     * ./.kdev4/simpleproject.kdev4
     *
     * Files are located in the OS temporary folder, subfolder 'simpleproject'
     */
    static KDevelop::IProject* GenerateSimpleProject();

    /**
     * This actually creates the same directory structure as GenerateSimpleProject().
     * However, the .kdev_include_paths file is also included in this project, so the structure
     * becomes:
     * ./simpleproject.kdev4
     * ./.kdev_include_paths
     * ./src/main.cpp
     * ./.kdev4/simpleproject.kdev4
     *
     * Files are located in the OS temporary folder, subfolder 'simpleproject'
     */
    static KDevelop::IProject* GenerateSimpleProjectWithOutOfProjectFiles();

    /**
     * Generates a pointer to a project with the following directory structure:
     * ./multipathproject.kdev4
     * ./src/main.cpp
     * ./anotherFolder/tst.h
     * ./.kdev4/multipathproject.kdev4
     *
     * Files are located in the OS temporary folder, subfolder 'multipathproject'
     */
    static KDevelop::IProject* GenerateMultiPathProject();

    /**
     * Generates a pointer to a project with the following directory structure:
     * ./emptyproject.kdev4
     * ./.kdev4/emptyproject.kdev4
     *
     * Files are located in the OS temporary folder, subfolder 'emptyproject'
     */
    static KDevelop::IProject* GenerateEmptyProject();

    /**
     * Generates a pointer to a project with the following directory structure:
     * ./emptybuilddirproject.kdev4
     * ./src/main.cpp
     * ./.kdev4/emptybuilddirproject.kdev4
     *
     * Files are located in the OS temporary folder, subfolder 'emptybuilddirproject'
     */
    static KDevelop::IProject* GenerateEmptyBuildDirProject();
};

}
#endif // PROJECTSGENERATOR_H
