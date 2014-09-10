/* This file is part of KDevelop
 *
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_IPROJECTPROVIDER_H
#define KDEVPLATFORM_IPROJECTPROVIDER_H

#include <QString>
#include <QWidget>
#include "interfacesexport.h"

class QUrl;

namespace KDevelop
{

class VcsLocationWidget;
class VcsLocation;
class VcsJob;

class KDEVPLATFORMINTERFACES_EXPORT IProjectProviderWidget : public QWidget
{
    Q_OBJECT
    public:
        IProjectProviderWidget(QWidget* parent = 0);

        /**
         * @returns a job that will create a working copy given the current state of the widget.
         *
         * @param destinationDirectory where the project will be downloaded.
         */
        virtual VcsJob* createWorkingCopy(const QUrl & destinationDirectory) = 0;

        /** @returns whether we have a correct location in the widget. */
        virtual bool isCorrect() const = 0;

    signals:
        void changed(const QString& name);
};

class KDEVPLATFORMINTERFACES_EXPORT IProjectProvider
{
    public:
        virtual ~IProjectProvider();

        virtual QString name() const = 0;

        virtual IProjectProviderWidget* providerWidget(QWidget* parent) = 0;
};

}
Q_DECLARE_INTERFACE( KDevelop::IProjectProvider, "org.kdevelop.IProjectProvider" )

#endif
