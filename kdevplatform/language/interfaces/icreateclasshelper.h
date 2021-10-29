/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
