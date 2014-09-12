/*  This file is part of KDevelop
    Copyright 2007 Alexander Dymo <adymo@kdevelop.org>
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

#ifndef KDEVPLATFORM_TEMPLATESMODEL_H
#define KDEVPLATFORM_TEMPLATESMODEL_H

#include <QStandardItemModel>

#include <language/languageexport.h>

namespace KDevelop
{

class TemplatesModelPrivate;

/**
 * @brief A convenience class for loading templates using .kdevtemplate files
 *
 * It loads template archives, extracts and stores their description files, and
 * displays them as a three-level tree structure.
 *
 * The locations for loading and storing files are determined by the typePrefix.
 * We use QStandardPaths with the GenericData type and create a filter string as such:
 * \li templates: typePrefix "/templates/"
 * \li descriptions: typePrefix "/template_descriptions/"
 * \li previews: typePrefix "/template_previews/"
 *
 * @sa ITemplateProvider::templatesModel()
 **/
class KDEVPLATFORMLANGUAGE_EXPORT TemplatesModel : public QStandardItemModel
{
    Q_OBJECT
public:

    /**
     * Extra roles for template-specific properties
     * @sa Qt::ItemDataRole
     **/
    enum TemplateRole
    {
        DescriptionFileRole = Qt::UserRole + 1, ///< Template description file name
        IconNameRole = Qt::UserRole + 2, ///< Template icon name
        CommentRole = Qt::UserRole + 3, ///< Template comment
        ArchiveFileRole = Qt::UserRole + 4 ///< Template archive file name
    };

    /**
     * Creates a new templates model
     *
     * @param typePrefix the type prefix used to determine resource locations.
     * @param parent parent object, defaults to 0.
     **/
    explicit TemplatesModel(const QString& typePrefix, QObject* parent = 0);

    /**
     * Destructor
     *
     **/
    virtual ~TemplatesModel();

    /**
     * Reloads all found templates
     **/
    virtual void refresh();

    /**
     * Loads template @p fileName and save it to the template storage directory.
     *
     * If the file is an archive, the whole archive will be copied.
     * If the file has a .desktop or .kdevtemplate suffix, the contents
     * of its containing directory will be archived and saved.
     **/
    QString loadTemplateFile(const QString& fileName);

    /**
     * Finds the model index of the template file @p fileName.
     *
     * For convenience, this function returns the found template index, as well as all of its ancestors.
     * The indexes are ordered from the top-level ancestor to the actual index of the template.
     * This is useful for managing selection when multiple views display different level of the model.
     *
     * @param fileName the template file name
     **/
    QModelIndexList templateIndexes(const QString& fileName) const;

    /**
     * Returns the type prefix used to find a template resource.
     **/
    QString typePrefix() const;

    /**
     * The model will include @p path during the search for template archives
     *
     * @param path Path to a directory that contains normal user data. The template model will search for a kdevappwizard/templates (or your model name prefix) directory
     * inside @p path and will use them. Please note that the path has to end with a '/'.
     */
    void addDataPath(const QString &path);

private:
    TemplatesModelPrivate* const d;
};

}

#endif // KDEVPLATFORM_TEMPLATESMODEL_H
