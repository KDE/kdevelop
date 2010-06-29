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

#ifndef IPROJECTPROVIDER_H
#define IPROJECTPROVIDER_H
#include <QString>

class QWidget;
class KUrl;

namespace KDevelop
{

class VcsLocationWidget;
class VcsLocation;
class VcsJob;

class IProjectProvider
{
    public:
        virtual QString name() const = 0;
        
        virtual VcsLocationWidget* vcsLocation(QWidget* parent) const=0;
        
        /**
         * @returns a job that will create a working copy for the specified location.
         * 
         * @param sourceRepository source of the checkout
         * @param destinationDirectory location of the created working copy (local repository)
         */
        virtual VcsJob* createWorkingCopy(const VcsLocation & sourceRepository, const KUrl & destinationDirectory) = 0;
};

Q_DECLARE_INTERFACE( KDevelop::IProjectProvider, "org.kdevelop.IProjectProvider" )

}
#endif
