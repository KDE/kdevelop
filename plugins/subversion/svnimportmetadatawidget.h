/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_SVNIMPORTMETADATAWIDGET_H
#define KDEVPLATFORM_PLUGIN_SVNIMPORTMETADATAWIDGET_H

#include <vcs/widgets/vcsimportmetadatawidget.h>

namespace Ui
{
    class SvnImportMetadataWidget;
}

class SvnImportMetadataWidget : public KDevelop::VcsImportMetadataWidget
{
    Q_OBJECT
public:
    SvnImportMetadataWidget( QWidget *parent );
    virtual ~SvnImportMetadataWidget();

    virtual void setSourceLocation( const KDevelop::VcsLocation& ) override;
    virtual void setSourceLocationEditable( bool ) override;
    virtual QUrl source() const override;
    virtual KDevelop::VcsLocation destination() const override;
    virtual QString message() const override;
    virtual void setUseSourceDirForDestination( bool ) override;
    virtual bool hasValidData() const override;
private:
    Ui::SvnImportMetadataWidget *m_ui;
    bool useSourceDirForDestination;
};

#endif
