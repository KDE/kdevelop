/***************************************************************************
Copyright 2006-2009 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LOCALPATCHSOURCE_H
#define LOCALPATCHSOURCE_H

#include <ksharedptr.h>
#include <QString>
#include <KUrl>

class LocalPatchSource : public KShared {

    ///This class should be used for identification instead of just the name, because the type of comparison might change in future
public:

    KUrl filename;
    KUrl baseDir;
    uint depth;
    QString command;

    LocalPatchSource()  : depth(0) {
    }
};

typedef KSharedPtr<LocalPatchSource> LocalPatchSourcePointer;

#endif // LOCALPATCHSOURCE_H
