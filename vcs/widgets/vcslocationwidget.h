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

#ifndef KDEVPLATFORM_VCSLOCATIONWIDGET_H
#define KDEVPLATFORM_VCSLOCATIONWIDGET_H

#include <QWidget>
#include "vcsexport.h"

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
        VcsLocationWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
        
        /** @returns the VcsLocation specified in the widget. */
        virtual VcsLocation location() const=0;
        
        /** @returns whether we have a correct location in the widget. */
        virtual bool isCorrect() const=0;
        
        /** @returns a proposed project name to be used as a hint for an identifier
         * for the VcsLocation.
         */
        virtual QString projectName() const=0;
        
    signals:
        void changed();
};

}
#endif


