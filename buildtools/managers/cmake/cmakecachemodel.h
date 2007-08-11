/* KDevelop CMake Support
 *
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
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

#ifndef CMAKECACHEMODEL_H
#define CMAKECACHEMODEL_H

#include <cmakeexport.h>
#include <QStandardItemModel>

#include <KUrl>

/** This class parses the CMakeCache.txt files and put it to a Model */
class KDEVCMAKECOMMON_EXPORT CMakeCacheModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        CMakeCacheModel(QObject* parent, const KUrl &path);
        ~CMakeCacheModel() {}

    private:
        KUrl m_filePath;
};

#endif
