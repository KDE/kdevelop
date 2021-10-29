/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "defaultcreateclasshelper.h"

#include <language/codegen/templateclassgenerator.h>

using namespace KDevelop;

DefaultCreateClassHelper::DefaultCreateClassHelper()
{

}

DefaultCreateClassHelper::~DefaultCreateClassHelper()
{

}

TemplateClassGenerator* DefaultCreateClassHelper::createGenerator(const QUrl& baseUrl)
{
    return new TemplateClassGenerator(baseUrl);
}

QList< DeclarationPointer > DefaultCreateClassHelper::defaultMethods(const QString& name) const
{
    Q_UNUSED(name);
    return QList<DeclarationPointer>();
}
