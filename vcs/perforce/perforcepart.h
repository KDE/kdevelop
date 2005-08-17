/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Extended to use perforce 2002 by Harald Fernengel <harry@kdevelop.org>*
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PERFORCEPART_H_
#define _PERFORCEPART_H_

#include <kdevversioncontrol.h>
#include <klocale.h>
//Added by qt3to4:
#include <Q3PopupMenu>

class Context;
class Q3PopupMenu;
class KAction;

class PerforcePart : public KDevVersionControl
{
    Q_OBJECT

public:
    PerforcePart( QObject *parent, const char *name, const QStringList & );
    ~PerforcePart();

    virtual QString shortDescription() const
    { return i18n( "Perforce is a version control system" ); }

    virtual void createNewProject(const QString& dir) {}
    virtual bool fetchFromRepository() { return true; }
    virtual KDevVCSFileInfoProvider *fileInfoProvider() const { return 0; }
    virtual bool isValidDirectory(const QString &dirPath) const { return true; }
    
private slots:
    void contextMenu(Q3PopupMenu *popup, const Context *context);
    void slotCommit();
    void slotUpdate();
    void slotAdd();
    void slotRemove();
    void slotEdit();
    void slotRevert();
    void slotDiff();

    void slotActionCommit();
    void slotActionUpdate();
    void slotActionAdd();
    void slotActionRemove();
    void slotActionEdit();
    void slotActionRevert();
    void slotActionDiff();

    void slotDiffFinished( const QString&, const QString& );

private:
    void commit( const QString& filename );
    void update( const QString& filename );
    void add( const QString& filename );
    void remove( const QString& filename );
    void edit( const QString& filename );
    void revert( const QString& filename );
    void diff( const QString& filename );
    QString currentFile();

    /** calls p4 with the command cmd and appends the filename */
    void execCommand( const QString& cmd, const QString& filename );
    void setupActions();
    QString popupfile;
    KAction *actionEdit, *actionRevert,
            *actionSubmit, *actionSync,
            *actionDiff, *actionAdd, *actionRemove;
};

#endif
