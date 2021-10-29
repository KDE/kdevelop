/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSIMPORTMETADATAWIDGET_H
#define KDEVPLATFORM_VCSIMPORTMETADATAWIDGET_H

#include <QWidget>
#include <vcs/vcsexport.h>

class QUrl;

namespace KDevelop
{

class VcsLocation;

class KDEVPLATFORMVCS_EXPORT VcsImportMetadataWidget : public QWidget
{
Q_OBJECT
public:
    explicit VcsImportMetadataWidget( QWidget* parent );
    ~VcsImportMetadataWidget() override;
    virtual QUrl source() const = 0;
    virtual VcsLocation destination() const = 0;
    virtual QString message() const = 0;

    /**
     * Check whether the given data is valid.
     * @returns true if all data in the widget is valid
     */
    virtual bool hasValidData() const = 0;
    
    /**
     * Select whether the widget should re-use the last part of the source location
     * for the destination. The default implementation simply ignores this setting.
     */
    virtual void setUseSourceDirForDestination( bool ) {}
Q_SIGNALS:
    void changed();
public Q_SLOTS:
    virtual void setSourceLocation( const VcsLocation& ) = 0;
    virtual void setSourceLocationEditable( bool ) = 0;
    virtual void setMessage(const QString& message) = 0;
};

}

#endif
