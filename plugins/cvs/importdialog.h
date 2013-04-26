/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_IMPORTDIALOG_H
#define KDEVPLATFORM_PLUGIN_IMPORTDIALOG_H

#include <KDialog>
#include <KUrl>
#include <KJob>

class ImportMetadataWidget;

class CvsPlugin;

/**
 * Asks the user for all options needed to import an existing directory into
 * a CVS repository
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class ImportDialog : public KDialog
{
    Q_OBJECT
public:
    ImportDialog(CvsPlugin *plugin, const KUrl& url, QWidget* parent=0);
    virtual ~ImportDialog();

public slots:
    virtual void accept();
    void jobFinished(KJob* job);

private:
    KUrl m_url;
    CvsPlugin* m_plugin;
    ImportMetadataWidget* m_widget;
};

#endif
