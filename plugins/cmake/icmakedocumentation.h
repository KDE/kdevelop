/*
    SPDX-FileCopyrightText: 2009 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ICMAKEDOCUMENTATION_H
#define ICMAKEDOCUMENTATION_H

#include <interfaces/idocumentationprovider.h>
#include <interfaces/idocumentation.h>

class QUrl;

class ICMakeDocumentation : public KDevelop::IDocumentationProvider
{
public:
    enum Type { Command, Variable, Module, Property, Policy, EOType };
    
    ~ICMakeDocumentation() override = default;
    virtual KDevelop::IDocumentation::Ptr description(const QString& identifier, const QUrl &file) const=0;
    virtual QVector<QString> names(Type t) const=0;
};

Q_DECLARE_INTERFACE( ICMakeDocumentation, "org.kdevelop.ICMakeDocumentation" )

#endif
