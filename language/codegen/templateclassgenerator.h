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

#ifndef KDEVELOP_TEMPLATECLASSGENERATOR_H
#define KDEVELOP_TEMPLATECLASSGENERATOR_H

#include "classgenerator.h"

namespace KDevelop
{

/**
 * Generates new classes from templates
 **/
class KDEVPLATFORMLANGUAGE_EXPORT TemplateClassGenerator : public ClassGenerator
{
public:
    /**
     * Creates a new generator.
     * 
     * You should call setTemplateDescription() before any other template-related functions.
     *
     * @param baseUrl the folder where new files will be created
     **/
    TemplateClassGenerator(const KUrl& baseUrl);
    ~TemplateClassGenerator();

    /**
     * @brief Selects the template to be used
     * 
     * This function must be called before using any other functions.
     *
     * The passed @p templateDescription should be an absolute path to a template description (.desktop) file.
     * TemplateClassGenerator will attempt to find a template archive with a matching name.
     *
     * @param templateDescription the template description file
     **/
    void setTemplateDescription(const QString& templateDescription);

    virtual DocumentChangeSet generate();
    virtual QStringList fileLabels();
    virtual QHash< QString, KUrl > fileUrlsFromBase (const KUrl& baseUrl, bool toLower = true);
    virtual QVariantHash templateVariables();

    /**
     * @return true if the template uses any custom options, false otherwise
     **/
    bool hasCustomOptions();

    /**
     * Returns the contents of the template's configuration options file.
     * The contents are in the format of .kcfg files used by KConfig XT.
     **/
    QByteArray customOptions();

    /**
     * Adds variables @p variables to the context passed to all template files.
     *
     * The variable values must be of a type registered with Grantlee::registerMetaType()
     *
     * @param variables additional variables to be passed to all templates
     **/
    void addVariables(const QVariantHash& variables);

    /**
     * Convenience function to render a string @p text as a Grantlee template
     *
     **/
    QString renderString(const QString& text);

private:
    class TemplateClassGeneratorPrivate* const d;
};

}

#endif // KDEVELOP_TEMPLATECLASSGENERATOR_H
