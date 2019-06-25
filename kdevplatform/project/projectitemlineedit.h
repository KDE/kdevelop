/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PROJECTITEMLINEEDIT_H
#define KDEVPLATFORM_PROJECTITEMLINEEDIT_H

#include <QLineEdit>
#include "projectexport.h"

namespace KDevelop { 
class ProjectBaseItem;
class IProject;
}
class ProjectItemLineEditPrivate;

class KDEVPLATFORMPROJECT_EXPORT ProjectItemLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit ProjectItemLineEdit(QWidget* parent=nullptr);
    ~ProjectItemLineEdit() override;

    /**
     * Sets this lineedit to show the given @p path, eventually removing
     * parts from the beginning if a base item is set
     * @note This should be preferred over using setText()
     */
    void setItemPath( const QStringList& path );
    
    /**
     * Generates a path from the content of the lineedit, including
     * the base item if present
     * @returns a path identifying the item selected in this lineedit
     */
    QStringList itemPath() const;
    
    /**
     * Sets @p item as the base item for this lineedit, the user
     * then doesn't need to specify the path leading to this item
     * and can just start typing the name of one of the subitems
     */
    void setBaseItem( KDevelop::ProjectBaseItem* item );
    
    /**
     * @returns the currently used base item
     */
    KDevelop::ProjectBaseItem* baseItem() const;
    
    KDevelop::ProjectBaseItem* currentItem() const;
    
    void setSuggestion(KDevelop::IProject* project);

public Q_SLOTS:
    bool selectItemDialog();
    
private Q_SLOTS:
    void showCtxMenu(const QPoint& p);
    
private:
    const QScopedPointer<class ProjectItemLineEditPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProjectItemLineEdit)
};

#endif
