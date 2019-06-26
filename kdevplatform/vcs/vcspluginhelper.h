/***************************************************************************
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_VCSPLUGINHELPER_H
#define KDEVPLATFORM_VCSPLUGINHELPER_H

#include "vcsexport.h"

#include <QUrl>

#include "vcsrevision.h"

class KJob;
class QMenu;

namespace KTextEditor
{
class View;
class Document;
}

// KTextEditor::AnnotationViewInterface has a bad signature in the
// annotationBorderVisibilityChanged signal, using type "View" instead
// of "KTextEditor::View".
// To enable a string based signal-slot connection, as needed due to
// annotationBorderVisibilityChanged being an "interface" signal,
// with the slot VcsPluginHelper::removeAnnotationModel,
// make View known here
using KTextEditor::View;

namespace KDevelop
{
class IPlugin;
class IBasicVersionControl;
class Context;
class VcsPluginHelperPrivate;

class KDEVPLATFORMVCS_EXPORT VcsPluginHelper
            : public QObject
{
    Q_OBJECT
public:
    VcsPluginHelper(IPlugin * parent, IBasicVersionControl * vcs);
    ~VcsPluginHelper() override;

    void setupFromContext(KDevelop::Context*);
    void addContextDocument(const QUrl& url);
    QList<QUrl> contextUrlList() const;
    /**
     * Creates and returns a menu with common actions.
     * Ownership of the actions in the menu stays with this VcsPluginHelper object.
     * @param parent the parent widget set for the QMenu for memory management
     */
    QMenu* commonActions(QWidget* parent);

public Q_SLOTS:
    void commit();
    void add();
    void revert();
    void history(const VcsRevision& rev = VcsRevision::createSpecialRevision( VcsRevision::Base ));
    void annotation();
    void annotationContextMenuAboutToShow( KTextEditor::View* view, QMenu* menu, int line);
    void diffToBase();
    void diffForRev();
    void diffForRevGlobal();
    void update();
    void pull();
    void push();
    void diffJobFinished(KJob* job);

    void revertDone(KJob* job);
    void disposeEventually(KTextEditor::Document*);
    void disposeEventually(View*, bool);

private Q_SLOTS:
    void delayedModificationWarningOn();
    // namespace-less type "View" needed here, see comment above on "using KTextEditor::View;"
    void handleAnnotationBorderVisibilityChanged(View* view, bool visible);

private:
    void diffForRev(const QUrl& url);

private:
    const QScopedPointer<class VcsPluginHelperPrivate> d_ptr;
    Q_DECLARE_PRIVATE(VcsPluginHelper)
};

} // namespace KDevelop

#endif
