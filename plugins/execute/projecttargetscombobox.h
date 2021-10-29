/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROJECTTARGETSCOMBOBOX_H
#define KDEVPLATFORM_PLUGIN_PROJECTTARGETSCOMBOBOX_H

#include <QComboBox>
#include <project/projectmodel.h>

namespace KDevelop {
class ProjectFolderItem;
}

class ProjectTargetsComboBox : public QComboBox
{
    Q_OBJECT
    public:
        explicit ProjectTargetsComboBox(QWidget* parent = nullptr);
        
        void setBaseItem(KDevelop::ProjectFolderItem* item, bool exec);
        void setCurrentItemPath(const QStringList& str);
        
        QStringList currentItemPath() const;
        
};

#endif // KDEVPLATFORM_PLUGIN_PROJECTTARGETSCOMBOBOX_H
