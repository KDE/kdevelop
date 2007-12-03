/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_CHECKOUTMETADATAWIDGET_H
#define SVN_CHECKOUTMETADATAWIDGET_H

#include <QWidget>

namespace Ui
{
class SvnCheckoutMetadataWidget;
}

namespace KDevelop
{
class VcsMapping;
}

class KUrl;

class SvnCheckoutMetadataWidget : public QWidget
{
    Q_OBJECT
public:
    SvnCheckoutMetadataWidget( QWidget *parent );
    virtual ~SvnCheckoutMetadataWidget();
    void setDestinationLocation( const KUrl& );
    KDevelop::VcsMapping mapping() const;
private:
    Ui::SvnCheckoutMetadataWidget* m_ui;
};

#endif
