/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPROJECTPROVIDER_H
#define KDEVPLATFORM_IPROJECTPROVIDER_H

#include <QWidget>
#include "interfacesexport.h"

class QUrl;

namespace KDevelop
{

class VcsJob;

class KDEVPLATFORMINTERFACES_EXPORT IProjectProviderWidget : public QWidget
{
    Q_OBJECT
    public:
        explicit IProjectProviderWidget(QWidget* parent = nullptr);

        /**
         * @returns a job that will create a working copy given the current state of the widget.
         *
         * @param destinationDirectory where the project will be downloaded.
         */
        virtual VcsJob* createWorkingCopy(const QUrl & destinationDirectory) = 0;

        /** @returns whether we have a correct location in the widget. */
        virtual bool isCorrect() const = 0;

    Q_SIGNALS:
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
