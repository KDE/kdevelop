/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _COMMITDIALOG_H_
#define _COMMITDIALOG_H_

//#include <qdialog.h>
#include <qstringlist.h>
#include "commitdialogbase.h"

class QTextEdit;
class QCheckBox;

class CommitDialog : public CommitDialogBase
{
    Q_OBJECT
public:
    CommitDialog( const QString &changeLogfileNamePath, QWidget *parent = 0 );

    //! Returns the text of the log
    QStringList logMessage() const;
    //! Returns true if the user requests the log message to be added to the general
    //! Changelog file
    bool mustAddToChangeLog() const;

    //! We need to set it when showing the dialog and then when saving the file
    void setChangeLogFileName( const QString &fileName );
    QString changeLogFileName() const;

protected slots:
    //! Override: must check for message not being void.
    virtual void accept();
};

#endif
