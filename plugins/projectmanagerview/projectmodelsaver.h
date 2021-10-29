/*
    SPDX-FileCopyrightText: 2012 Andrew Fuller <mactalla.obair@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROJECTMODELSAVER_H
#define KDEVPLATFORM_PLUGIN_PROJECTMODELSAVER_H

#include <KConfigViewStateSaver>

namespace KDevelop
{

class IProject;

class ProjectModelSaver: public KConfigViewStateSaver
{
    Q_OBJECT
public:
    ProjectModelSaver();

    /**
     * If @p project is non-null, only files from that project will
     * be handled by @c indexFromConfigString() and @c indexToConfigString().
     *
     * For other files, an invalid index or empty string is be returned.
     */
    void setProject(IProject* project);

    QModelIndex indexFromConfigString(const QAbstractItemModel *model, const QString &key) const override;

    QString indexToConfigString(const QModelIndex &index) const override;

private:
    IProject* m_project = nullptr;
};

}

#endif // KDEVPLATFORM_PLUGIN_PROJECTMODELSAVER_H
