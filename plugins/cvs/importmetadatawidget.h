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
    explicit ImportMetadataWidget(QWidget* parent=nullptr);
    ~ImportMetadataWidget() override;

    QUrl source() const override;
    KDevelop::VcsLocation destination() const override;
    QString message() const override;
    void setSourceLocation( const KDevelop::VcsLocation& ) override;
    void setSourceLocationEditable( bool ) override;
    void setMessage(const QString& message) override;
    bool hasValidData() const override;

private:
    Ui::ImportMetadataWidget* m_ui;
};

#endif
