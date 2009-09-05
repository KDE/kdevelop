/*
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

#ifndef KDEVELOP_IPATCHSOURCE_H
#define KDEVELOP_IPATCHSOURCE_H

#include <ksharedptr.h>
#include <kurl.h>
#include <qpointer.h>
#include <interfaces/iextension.h>
#include "interfacesexport.h"

namespace KDevelop {

///Any entity may delete an IPatchSource based object at will, so it must always be referenced through a QPointer.
class KDEVPLATFORMINTERFACES_EXPORT IPatchSource : public QObject {
    Q_OBJECT
    public:
        typedef QPointer<IPatchSource> Ptr;
        
        ///Name of the patch, that will be shown in a combo box. Should describe the patch in a useful way, for example "Difference to base in kdevplatform/language"
        virtual QString name() const = 0;
        
        ///Icon that will be shown with the patch
        virtual QIcon icon() const;
        
        ///Explicit updating of the patch: If it is a dynamic patch, it should re-compare the files or whatever needs to be done
        ///If the patch has changed, patchChanged needs to be emitted
        virtual void update() = 0;
        
        ///Name of the patch file
        virtual KUrl file() const = 0;
        
        ///Should return the base-dir of the patch
        virtual KUrl baseDir() const = 0;
        
        ///Called when the user has reviewed and accepted this patch
        virtual void finishReview();
        
        ///Called when the user has rejected this patch
        virtual void cancelReview();
        
        ///Should return whether the user may cancel this review (cancelReview will be called when he does)
        virtual bool canCancel() const;
        
    Q_SIGNALS:
        ///Should be emitted whenever the patch has changed.
        void patchChanged();
};

class KDEVPLATFORMINTERFACES_EXPORT IPatchReview {
    public:
        virtual ~IPatchReview();
        
        enum ReviewMode {
            OpenAndRaise //Opens the related files in the review area, switches to that area, and raises the patch-review toolview
        };
        ///Starts a review on the patch: Opens the patch and the files within the review area
        virtual void startReview(IPatchSource* patch, ReviewMode mode = OpenAndRaise) = 0;
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS(KDevelop, IPatchReview, "org.kdevelop.IPatchReview")
Q_DECLARE_INTERFACE(KDevelop::IPatchReview, "org.kdevelop.IPatchReview")

#endif // KDEVELOP_IPATCHSOURCE_H
