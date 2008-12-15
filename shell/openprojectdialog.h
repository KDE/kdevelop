/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OPENPROJECTDIALOG_H
#define OPENPROJECTDIALOG_H

#include <kassistantdialog.h>
#include <kio/udsentry.h>

class KPageWidgetItem;
namespace KIO
{
class Job;
}

namespace KDevelop
{

class OpenProjectDialog : public KAssistantDialog
{
Q_OBJECT
public:
    OpenProjectDialog( QWidget* parent = 0 );
private slots:
    void validateOpenUrl( const KUrl& );
    void gotFileList( KIO::Job*, const KIO::UDSEntryList& list );
private:
    KUrl directory;
    QStringList entriesList;
    KPageWidgetItem* openPage;
    KPageWidgetItem* projectInfoPage;
};

}

#endif
