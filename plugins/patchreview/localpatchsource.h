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

#ifndef KDEVPLATFORM_PLUGIN_LOCALPATCHSOURCE_H
#define KDEVPLATFORM_PLUGIN_LOCALPATCHSOURCE_H

#include <QString>
#include <KUrl>
#include <interfaces/ipatchsource.h>

class LocalPatchSource : public KDevelop::IPatchSource
{
    Q_OBJECT
public:
    LocalPatchSource();
    virtual ~LocalPatchSource();

    virtual QString name() const;

    virtual KUrl baseDir() const {
        return m_baseDir;
    }

    virtual KUrl file() const {
        return m_filename;
    }

    virtual uint depth() const {
        return m_depth;
    }

    virtual void update();
    virtual QIcon icon() const;

    void setFilename(const KUrl& filename) { m_filename = filename; }
    void setBaseDir(const KUrl& dir) { m_baseDir = dir; }
    void setCommand(const QString& cmd) { m_command = cmd; }

    QString command() const { return m_command; }
    
    virtual bool isAlreadyApplied() const { return m_applied; }

    void setAlreadyApplied( bool applied ) { m_applied = applied; }

private:
    KUrl m_filename;
    KUrl m_baseDir;
    QString m_command;
    bool m_applied;
    uint m_depth;
};

#endif // KDEVPLATFORM_PLUGIN_LOCALPATCHSOURCE_H
