/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEMPLATECLASSGENERATOR_H
#define KDEVPLATFORM_TEMPLATECLASSGENERATOR_H

#include <QHash>
#include <QVariantHash>

#include <language/languageexport.h>
#include <language/duchain/duchainpointer.h>

class QUrl;

namespace KTextEditor {
class Cursor;
}

namespace KDevelop {
struct ClassDescription;
class TemplateRenderer;
class SourceFileTemplate;
class DocumentChangeSet;
class TemplateClassGeneratorPrivate;

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
 * @li @c namespaces (QStringList) - the list of nested namespaces in which the class will be declared
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
class KDEVPLATFORMLANGUAGE_EXPORT TemplateClassGenerator
{
public:
    using UrlHash = QHash<QString, QUrl>;

    /**
     * Creates a new generator.
     *
     * You should call setTemplateDescription() before any other template-related functions.
     *
     * @param baseUrl the folder where new files will be created
     **/
    explicit TemplateClassGenerator(const QUrl& baseUrl);
    virtual ~TemplateClassGenerator();

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
    void setTemplateDescription(const SourceFileTemplate& templateDescription);

    /**
     * Set the name (without namespace) for this class
     */
    void setName(const QString&);

    /**
     * \return The name of the class to generate (excluding namespaces)
     */
    QString name() const;

    /**
     * \param identifier The Qualified identifier that the class will have
     */
    virtual void setIdentifier(const QString& identifier);

    /**
     * \return The Identifier of the class to generate (including all used namespaces)
     */
    virtual QString identifier() const;

    /**
     * \param namespaces The list of nested namespaces in which this class is to be declared
     */
    virtual void setNamespaces(const QStringList& namespaces);

    /**
     * \return The list of nested namespace in which this class will be declared
     */
    virtual QStringList namespaces() const;

    void addBaseClass(const QString& base);
    void setBaseClasses(const QList<QString>& bases);
    QList<DeclarationPointer> directBaseClasses() const;
    QList<DeclarationPointer> allBaseClasses() const;

    void setLicense(const QString& license);
    QString license() const;

    void setDescription(const ClassDescription& description);
    ClassDescription description() const;

    virtual DocumentChangeSet generate();

    QHash<QString, QString> fileLabels() const;

    QUrl baseUrl() const;
    UrlHash fileUrls() const;

    void setFileUrl(const QString& outputFile, const QUrl& url);
    QUrl fileUrl(const QString& outputFile) const;

    void setFilePosition(const QString& outputFile, const KTextEditor::Cursor& position);
    KTextEditor::Cursor filePosition(const QString& outputFile) const;

    SourceFileTemplate sourceFileTemplate() const;

    /**
     * Adds variables @p variables to the context passed to all template files.
     *
     * The variable values must be of a type registered with KTextTemplate::registerMetaType()
     *
     * @param variables additional variables to be passed to all templates
     **/
    void addVariables(const QVariantHash& variables);

    /**
     * Convenience function to render a string @p text as a KTextTemplate template
     **/
    QString renderString(const QString& text) const;

    /**
     * The template renderer used to render all the templates for this class.
     *
     * This function is useful if you want a rendeder with all current template variables.
     */
    TemplateRenderer* renderer() const;

private:
    const QScopedPointer<class TemplateClassGeneratorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(TemplateClassGenerator)
};
}

#endif // KDEVPLATFORM_TEMPLATECLASSGENERATOR_H
