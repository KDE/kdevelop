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

#ifndef KDEVELOP_TEMPLATESMODEL_H
#define KDEVELOP_TEMPLATESMODEL_H

#include <QStandardItemModel>

#include "../languageexport.h"

class KComponentData;
namespace KDevelop
{

class TemplatesModelPrivate;

/**
 * @brief A convenience class for loading templates using .kdevtemplate files
 *
 * It loads template archives, extracts and stores their description files, and
 * displays them as a three-level tree structure.
 *
 * The locations for loading and storing files are determined from the componentData()
 * using resource types set by setDescriptionResourceType() and setTemplateResourceType().
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
        CommentRole = Qt::UserRole + 3 ///< Template comment
    };

    /**
     * Creates a new templates model
     *
     * @param componentData the component data used for determining resource locations
     * @param parent parent object, defaults to 0.
     **/
    explicit TemplatesModel(const KComponentData& componentData, QObject* parent = 0);

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
    QModelIndexList templateIndexes(const QString& fileName);

    /**
     * Sets the resource type for template descriptions to @p type
     *
     * @param type the new resource type for template descriptions
     * @sa descriptionResourceType(), KStandardDirs
     **/
    void setDescriptionResourceType(const QByteArray& type);

    /**
     * Returns the current resource type for template descriptions
     *
     * @sa setDescriptionResourceType()
     **/
    QByteArray descriptionResourceType() const;

    /**
     * Sets the resource type for template previews to @p type
     *
     * @param type the new resource type for template previews
     * @sa previewResourceType(), KStandardDirs
     **/
    void setPreviewResourceType(const QByteArray& type);

    /**
     * Returns the current resource type for template previews
     *
     * @sa setPreviewResourceType()
     **/
    QByteArray previewResourceType() const;

    /**
     * Sets the resource type for template archives to @p type
     *
     * @param type the new resource type for template archives
     * @sa templateResourceType(), KStandardDirs
     **/
    void setTemplateResourceType(const QByteArray& type);
    /**
     * Returns the current resource type for template archives
     *
     * @sa setTemplateResourceType()
     **/
    QByteArray templateResourceType() const;

    /**
     * @return the component data used by this model.
     **/
    KComponentData componentData();

protected:
    /**
     * Extracts description files from all available template archives and saves them to a location
     * determined by descriptionResourceType().
     **/
    void extractTemplateDescriptions();

    /**
     * Checks whether the template @p descriptionName exists.
     *
     * @param descriptionName the name of the stored template description (.kdevtemplate) file
     * @return true if a matching template file exists, false otherwise
     **/
    bool templateExists(const QString& descriptionName);

    /**
     * Creates a model item for the template @p name in category @p category
     *
     * @param name the name of the new template
     * @param category the category of the new template
     * @return the created item
     **/
    QStandardItem *createItem(const QString &name, const QString &category);

private:
    TemplatesModelPrivate* const d;
};

}

#endif // KDEVELOP_TEMPLATESMODEL_H
