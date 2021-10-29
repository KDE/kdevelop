/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPROJECTFILTERPROVIDER_H
#define KDEVPLATFORM_IPROJECTFILTERPROVIDER_H

#include <project/projectexport.h>

#include <QObject>
#include <QSharedPointer>

namespace KDevelop
{

class IProject;
class IProjectFilter;

/**
 * @short An interface to create project filters.
 *
 * Plugins implement this interface and return an instance of a custom
 * IProjectFilter implementation, which is then doing the actual filtering.
 *
 * The filter should then allow filtering of files and folders in a
 * threadsafe way.
 *
 * Project managers are supposed to listen to the filterChanged signal
 * and update the project filter accordingly, i.e. by creating a new
 * filter instance.
 *
 * This interface was designed such that project filtering can be done in a
 * multithreaded project manager without the need for any locks.
 *
 * @author Milian Wolff
 */
class KDEVPLATFORMPROJECT_EXPORT IProjectFilterProvider
{
public:
    virtual ~IProjectFilterProvider();

    /**
     * Factory function to create threadsafe project filter instances.
     *
     * @param project The project that the filter should operate on.
     *
     * @return A threadsafe implementation of the IProjectFilter instance.
     */
    virtual QSharedPointer<IProjectFilter> createFilter(IProject* project) const = 0;

Q_SIGNALS:
    /**
     * Should be emitted whenever the filter rules for the given @p project changed.
     *
     * Listeners should act on this signal by replacing their current filter
     * with a new instance.
     */
    void filterChanged(KDevelop::IProjectFilterProvider*, KDevelop::IProject* project);
};

}

Q_DECLARE_INTERFACE( KDevelop::IProjectFilterProvider, "org.kdevelop.IProjectFilterProvider")

#endif // KDEVPLATFORM_IPROJECTFILTERPROVIDER_H
