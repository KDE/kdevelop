/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VCSEVENTWIDGET_H
#define KDEVPLATFORM_VCSEVENTWIDGET_H

#include <QWidget>
#include <vcs/vcsexport.h>

class QUrl;

namespace KDevelop
{
class VcsRevision;
class IBasicVersionControl;
class VcsEventWidgetPrivate;

class KDEVPLATFORMVCS_EXPORT VcsEventWidget : public QWidget
{
    Q_OBJECT
public:
    VcsEventWidget( const QUrl& url, const VcsRevision& rev, KDevelop::IBasicVersionControl* iface, QWidget* parent = nullptr );
    ~VcsEventWidget() override;

private:
    const QScopedPointer<class VcsEventWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(VcsEventWidget)
};
}

#endif
