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
#include <interfaces/ipatchsource.h>
#include <klocalizedstring.h>

class LocalPatchSource : public KDevelop::IPatchSource {
Q_OBJECT
public:

    virtual QString name() const {
        return i18n("Custom Patch");
    }
    
    virtual KUrl baseDir() const {
        return m_baseDir;
    }

    virtual KUrl file() const {
        return m_filename;
    }
    
    virtual void update();
    
    KUrl m_filename;
    KUrl m_baseDir;
    uint m_depth;
    QString m_command;
    virtual bool isAlreadyApplied() const { return false; }

    LocalPatchSource()  : m_depth(0) {
    }
    virtual ~LocalPatchSource();
};

#endif // LOCALPATCHSOURCE_H
