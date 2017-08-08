/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_ICREATECLASSHELPER_H
#define KDEVPLATFORM_ICREATECLASSHELPER_H

#include <language/languageexport.h>
#include <language/duchain/duchainpointer.h>

class QUrl;

namespace KDevelop {

class TemplateClassGenerator;

/**
 * @brief A language-specific helper interface for creating new classes.
 *
 * This interface contains methods that the "create class" dialog cannot determine
 * by itself and which cannot be specified in a template.
 *
 * They are mostly taken from CreateClassAssistant.
 **/
class KDEVPLATFORMLANGUAGE_EXPORT ICreateClassHelper
{
public:
    virtual ~ICreateClassHelper();

    /**
     * @return a new class generator.
     *
     * The caller takes ownership of the returned object.
     **/
    virtual TemplateClassGenerator* createGenerator(const QUrl& url) = 0;

    /**
     * Returns a list of suggested default methods for a new class named @p name.
     * The user will be able to select the ones for including in the class.
     *
     * @param name the name of the new class
     * @return a list of default methods for the new class
     */
    virtual QList<DeclarationPointer> defaultMethods(const QString& name) const = 0;
};

}

#endif // KDEVPLATFORM_ICREATECLASSHELPER_H
