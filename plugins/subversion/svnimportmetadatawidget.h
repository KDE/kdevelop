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

#ifndef SVNIMPORTMETADATAWIDGET_H
#define SVNIMPORTMETADATAWIDGET_H

#include <vcs/widgets/vcsimportmetadatawidget.h>

namespace Ui
{
    class SvnImportMetadataWidget;
}

namespace KDevelop
{
class VcsLocation;
}

class SvnImportMetadataWidget : public KDevelop::VcsImportMetadataWidget
{
    Q_OBJECT
public:
    SvnImportMetadataWidget( QWidget *parent );
    virtual ~SvnImportMetadataWidget();

    virtual void setSourceLocation( const KDevelop::VcsLocation& );
    virtual void setSourceLocationEditable( bool );
    virtual KDevelop::VcsMapping mapping() const;
    virtual QString message() const;

private:
    Ui::SvnImportMetadataWidget *m_ui;
};

#endif
