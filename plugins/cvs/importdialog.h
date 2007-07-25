/***************************************************************************
 *   Copyright 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include <KUrl>
#include <KJob>

#include "ui_importdialog.h"

class CvsPart;

/**
 * Asks the user for all options needed to import an existing directory into
 * a CVS repository
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class ImportDialog : public KDialog, private Ui::ImportDialogBase
{
    Q_OBJECT
public:
    ImportDialog(CvsPart *part, const KUrl& url, QWidget* parent=0);
    virtual ~ImportDialog();

public slots:
    virtual void accept();
    void jobFinished(KJob* job);

private:
    KUrl m_url;
    CvsPart* m_part;
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
