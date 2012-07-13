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

class SourceFileTemplate;

/**
 * Generates new classes from templates
 * 
 *
 * @section Variables Variables Passed to Templates
 * 
 * TemplateClassGenerator makes use of the ClassDescription returned by ClassGenerator::description(). 
 * From this description, it constructs the following variables:
 * @li @c description (ClassDescription) - the class description
 * @li @c name (QString) - the class name, same as @c description.name
 * @li @c identifier (QString) - the full class identifier, composed of namespaces and name
 * @li @c members (VariableDescriptionList) - data members, same as @c description.members
 * @li @c functions (FunctionDescriptionList) - function members, same as @c description.methods
 * @li @c base_classes (InheritanceDescriptionList) - directly inherited classes, same as @c description.baseClasses
 * @li @c license (QString) - the license for this class, including author copyright, without comment characters or indentation. It is recommended to use the "lines_prepend" filters from library "kdev_filters" to format it.
 * 
 * For each output file, TemplateRenderer add two variables named @c output_file_x
 * and @c output_file_x_absolute, where @c x is replaced
 * with the file name specified in the template description file.
 * See TemplateRenderer::renderFileTemplate() for details. 
 * 
 * If the templates uses custom options, these options are added to the template variables. Their names match the
 * names specified in the options file, and their values to the values entered by the user. 
 * 
 * Subclasses can override templateVariables() and insert additional variables. 
 * 
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
    virtual QHash<QString,QString> fileLabels();
    virtual QHash< QString, KUrl > fileUrlsFromBase (const KUrl& baseUrl, bool toLower = true);
    /**
     * @brief Default variables that will be passed to templates
     * 
     * These are populated from description(). Subclasses can override this function
     * to provide additional variables, but should always call the base implementation, like this
     * 
     * @code
     * MyGenerator::templateVariables()
     * {
     *     QVariantHash variables = TemplateClassGenerator::templateVariables();
     *     variables["my_variable"] = "SomeString";
     *     return variables;
     * }
     * @endcode
     *
     **/
    virtual QVariantHash templateVariables();

    SourceFileTemplate* sourceFileTemplate();

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
