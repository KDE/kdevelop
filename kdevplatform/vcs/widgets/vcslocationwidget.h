/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSLOCATIONWIDGET_H
#define KDEVPLATFORM_VCSLOCATIONWIDGET_H

#include <QWidget>
#include <vcs/vcsexport.h>

namespace KDevelop
{
class VcsLocation; 

/**
 * Provides a widget to be used to ask the user for a VersionControl location.
 * 
 * Every VCS plugin will provide their own to be able to construct VcsLocations
 * from the UI in a VCS-dependent fashion.
 */
class KDEVPLATFORMVCS_EXPORT VcsLocationWidget : public QWidget
{
    Q_OBJECT
    public:
        explicit VcsLocationWidget(QWidget* parent = nullptr, Qt::WindowFlags f = {});
        
        /** @returns the VcsLocation specified in the widget. */
        virtual VcsLocation location() const=0;
        
        /** @returns whether we have a correct location in the widget. */
        virtual bool isCorrect() const=0;
        
        /** @returns a proposed project name to be used as a hint for an identifier
         * for the VcsLocation.
         */
        virtual QString projectName() const=0;

        /** Sets the location by a respective URL
         * @param remoteLocation the URL used to access a remote repository
         * @see IBasicVersionControl::isValidRemoteRepositoryUrl
         */
        virtual void setLocation(const QUrl& remoteLocation) = 0;

    Q_SIGNALS:
        void changed();
};

}
#endif


