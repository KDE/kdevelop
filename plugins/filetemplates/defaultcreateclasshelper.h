/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_DEFAULTCREATECLASSHELPER_H
#define KDEVPLATFORM_PLUGIN_DEFAULTCREATECLASSHELPER_H

#include <language/interfaces/icreateclasshelper.h>

/**
 * A default class creation helper.
 *
 * This is used when no custom helper is supplied by the language plugin.
 */
class DefaultCreateClassHelper : public KDevelop::ICreateClassHelper
{
public:
    DefaultCreateClassHelper();
    ~DefaultCreateClassHelper() override;

    /**
     * @return a new @c TemplateClassGenerator.
     */
    KDevelop::TemplateClassGenerator* createGenerator(const QUrl& baseUrl) override;
    /**
     * @return an empty list of methods.
     */
    QList< KDevelop::DeclarationPointer > defaultMethods(const QString& name) const override;
};

#endif // KDEVPLATFORM_PLUGIN_DEFAULTCREATECLASSHELPER_H
