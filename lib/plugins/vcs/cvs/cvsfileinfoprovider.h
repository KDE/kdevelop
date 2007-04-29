/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVSFILEINFOPROVIDER_H
#define CVSFILEINFOPROVIDER_H

#include <QObject>
#include <KUrl>
#include <KJob>

#include <iversioncontrol.h>

class CvsProxy;

/**
 * The CvsFileInfoProvider allows to request status information about
 * files in a CVS managed directory.
 * 
 * The retrieving of the file infos can be done either async of sync.
 * @see requestStatusASync()
 * @see requestStatusSync()
 * 
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsFileInfoProvider : public QObject
{
    Q_OBJECT
public:
    CvsFileInfoProvider(CvsProxy* proxy, QObject* parent=0);
    virtual ~CvsFileInfoProvider();

    /**
     * Retrieves the file infos async.
     * @param directory This directory will be searched for CVS managed files
     * @return true if the job has been started. In such a case the caller needs to 
     *         connect to the statusReady() to be informed when the operation finished
     */
    bool requestStatusASync( const KUrl& directory, KDevelop::IVersionControl::WorkingMode mode );

    /**
     * Retrieves the file infos in sync.
     * @param directory This directory will be searched for CVS managed files
     * @return a list of status information about the files found in the given directory.
     */
    QList<KDevelop::VcsFileInfo> requestStatusSync( const KUrl& directory, KDevelop::IVersionControl::WorkingMode mode );

    /**
     * Utility function to parse the output of the "cvs status" job for
     * the file infos.
     * @param output The output of a "cvs status" job
     * @param infos All found status information will be added to this list
     */
    void parseOutput(const QString& output, QList<KDevelop::VcsFileInfo>& infos);

signals:
    /**
     * Gets emitted when an async operation finished.
     * @param infos All found status information
     */
    void statusReady(const QList<KDevelop::VcsFileInfo> &infos);

private slots:
    void slotJobFinished(KJob* job);

private:
    KDevelop::VcsFileInfo::VcsFileState String2EnumState(QString stateAsString);

    CvsProxy* m_proxy;
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
