/***************************************************************************
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VCSPLUGINHELPER_H
#define VCSPLUGINHELPER_H

#include "vcsexport.h"

#include <kurl.h>
#include <memory>

class KJob;
class QAction;
class QActionGroup;
class QMenu;

namespace KDevelop
{
class VcsDiff;
class IPlugin;
class IBasicVersionControl;
class Context;
class ContextMenuExtension;
class VcsCommitDialog;
class ProjectBaseItem;

class KDEVPLATFORMVCS_EXPORT VcsPluginHelper
            : public QObject
{
    Q_OBJECT
public:
    VcsPluginHelper(IPlugin * parent, IBasicVersionControl * vcs);
    virtual ~VcsPluginHelper();

    void setupFromContext(KDevelop::Context*);
    KUrl::List const & contextUrlList();
    QMenu* commonActions();

private Q_SLOTS:
    void commit();
    void add();
    void revert();
    void history();
    void annotation();
    void diffToHead();
    void diffToBase();
    void update();
    void executeCommit(KDevelop::VcsCommitDialog* dlg);
    void cancelCommit(KDevelop::VcsCommitDialog* dlg);
    void diffJobFinished(KJob* job);
private:
    KUrl urlForItem(KDevelop::ProjectBaseItem* item) const;

    struct VcsPluginHelperPrivate;
    std::auto_ptr<VcsPluginHelperPrivate> d;
};

} // namespace KDevelop

#endif
