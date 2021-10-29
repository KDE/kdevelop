/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSDIFFWIDGET_H
#define KDEVPLATFORM_VCSDIFFWIDGET_H

#include <QWidget>
#include <vcs/vcsexport.h>

namespace KDevelop
{
class VcsJob;
class VcsRevision;
class VcsDiffWidgetPrivate;

class KDEVPLATFORMVCS_EXPORT VcsDiffWidget : public QWidget
{
Q_OBJECT
public:
    explicit VcsDiffWidget( KDevelop::VcsJob*, QWidget* parent = nullptr );
    ~VcsDiffWidget() override;
    void setRevisions( const KDevelop::VcsRevision&, const KDevelop::VcsRevision& );
    
private:
    const QScopedPointer<class VcsDiffWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(VcsDiffWidget)
};

}

#endif
