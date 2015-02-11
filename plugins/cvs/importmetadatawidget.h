/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_IMPORTMETADATAWIDGET_H
#define KDEVPLATFORM_PLUGIN_IMPORTMETADATAWIDGET_H

#include <vcs/widgets/vcsimportmetadatawidget.h>

#include "ui_importmetadatawidget.h"

class CvsPlugin;

namespace KDevelop
{
class VcsLocation;
}

/**
 * Asks the user for all options needed to import an existing directory into
 * a CVS repository
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class ImportMetadataWidget : public KDevelop::VcsImportMetadataWidget, private Ui::ImportMetadataWidget
{
    Q_OBJECT
public:
    ImportMetadataWidget(QWidget* parent=0);
    virtual ~ImportMetadataWidget();

    virtual QUrl source() const override;
    virtual KDevelop::VcsLocation destination() const override;
    virtual QString message() const override;
    virtual void setSourceLocation( const KDevelop::VcsLocation& ) override;
    virtual void setSourceLocationEditable( bool ) override;
    virtual bool hasValidData() const override;

private:
    Ui::ImportMetadataWidget* m_ui;
};

#endif
