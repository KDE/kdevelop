/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPROJECTCONFIGSKELETON_H
#define KDEVPROJECTCONFIGSKELETON_H

#include "kdevexport.h"
#include <kconfigskeleton.h>

namespace KDevelop
{

class KDEVPLATFORM_EXPORT ProjectConfigSkeleton: public KConfigSkeleton
{
    Q_OBJECT
public:

    /**
     * Constructs a new skeleton, the skeleton will write to the project
     * configuration file, which is by default located in projectdir/.kdev4
     * The defaults will be set from the project file, which is in the projectdir
     *
     * @param configname The absolute filename of the project configuration file
     */
    ProjectConfigSkeleton( const QString & configname );

    virtual ~ProjectConfigSkeleton();

    void setDeveloperTempFile( const QString& );
    void setProjectFileUrl( const QString& );
    void setDeveloperFileUrl( const QString& );
protected:
    void usrSetDefaults();
    bool usrUseDefaults( bool b );
    void usrWriteConfig();
private:
    /**
     * There's no way in KDE4 API to find out the file that the config object
     * was created from, so we can't apply defaults when using this
     * constructory. Thus I'm making this private, so we can find out when
     * this constructor is used and see if we need to add apropriate API to
     * kdelibs
     */
     ProjectConfigSkeleton( KSharedConfig::Ptr config );
     struct ProjectConfigSkeletonPrivate * const d;
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
