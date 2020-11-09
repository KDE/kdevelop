/*
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
     * This actually does create the same directory structure as genereate simple project.
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
};

}
#endif // PROJECTSGENERATOR_H
