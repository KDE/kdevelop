/*
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_IPATCHSOURCE_H
#define KDEVPLATFORM_IPATCHSOURCE_H

#include <QUrl>

#include <qpointer.h>
#include "vcsexport.h"
#include <vcs/vcsstatusinfo.h>

namespace KDevelop {

///Any entity may delete an IPatchSource based object at will, so it must always be referenced through a QPointer (Just use IPatchSource::Ptr).
class KDEVPLATFORMVCS_EXPORT IPatchSource : public QObject
{
    Q_OBJECT
    public:
        typedef QPointer<IPatchSource> Ptr;

        ///Name of the patch, that will be shown in a combo box. Should
        ///describe the patch in a useful way, for example "Difference to base in kdevplatform/language"
        virtual QString name() const = 0;

        ///Icon that will be shown with the patch
        virtual QIcon icon() const;

        ///Should tell if the patch is already applied on the local version.
        virtual bool isAlreadyApplied() const = 0;

        ///Explicit updating of the patch: If it is a dynamic patch, it
        ///should re-compare the files or whatever needs to be done
        ///If the patch has changed, patchChanged needs to be emitted
        virtual void update() = 0;

        ///Name of the patch file
        virtual QUrl file() const = 0;

        ///Should return the base-dir of the patch
        virtual QUrl baseDir() const = 0;

        ///Can return a custom widget that should be shown to the user with this patch
        ///The ownership of the widget is shared between the caller and the patch-source (both may delete it at will)
        ///The default implementation returns zero
        virtual QWidget* customWidget() const;

        ///May return a custom text for the "Finish Review" action.
        ///The default implementation returns QString(), which means that the default is used
        virtual QString finishReviewCustomText() const;

        ///Called when the user has reviewed and accepted this patch
        ///If canSelectFiles() returned true, @p selection will contain the list of selected files
        ///If this returns false, the review is not finished.
        virtual bool finishReview(QList<QUrl> selection);

        ///Called when the user has rejected this patch
        virtual void cancelReview();

        ///Should return whether the user may cancel this review (cancelReview will be called when he does)
        ///The default implementation returns false
        virtual bool canCancel() const;

        ///Should return whether the user should be able to select files of the patch
        ///The files available for selection will be all files affected by the patch, and the files
        ///return by additionalSelectableFiles() The default implementation returns false
        virtual bool canSelectFiles() const;

        ///May return an additional list of selectable files together with short description strings for this patch
        ///The default implementation returns an empty list
        virtual QMap<QUrl, KDevelop::VcsStatusInfo::State> additionalSelectableFiles() const;

        /// Depth - number of directories to left-strip from paths in the patch - see "patch -p"
        /// Defaults to 0
        virtual uint depth() const;
    Q_SIGNALS:
        ///Should be emitted whenever the patch has changed.
        void patchChanged();
};

class KDEVPLATFORMVCS_EXPORT IPatchReview
{
    public:
        virtual ~IPatchReview();

        enum ReviewMode {
            OpenAndRaise //Opens the related files in the review area, switches to that area, and raises the patch-review toolview
        };
        ///Starts a review on the patch: Opens the patch and the files within the review area
        virtual void startReview(IPatchSource* patch, ReviewMode mode = OpenAndRaise) = 0;
};

}

Q_DECLARE_INTERFACE(KDevelop::IPatchReview, "org.kdevelop.IPatchReview")

#endif // KDEVPLATFORM_IPATCHSOURCE_H
