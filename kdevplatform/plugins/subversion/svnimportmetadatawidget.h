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
    explicit SvnImportMetadataWidget( QWidget *parent );
    ~SvnImportMetadataWidget() override;

    void setSourceLocation( const KDevelop::VcsLocation& ) override;
    void setSourceLocationEditable( bool ) override;
    QUrl source() const override;
    KDevelop::VcsLocation destination() const override;
    QString message() const override;
    void setUseSourceDirForDestination( bool ) override;
    void setMessage(const QString& message) override;
    bool hasValidData() const override;
private:
    Ui::SvnImportMetadataWidget *m_ui;
    bool useSourceDirForDestination;
};

#endif
