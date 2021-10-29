/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
