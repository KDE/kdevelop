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

#include "patchdocument.h"
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/ipartcontroller.h>
#include <QLabel>
#include <KParts/Part>
#include <kompare/kompareinterface.h>
#include <vcs/vcsdiff.h>
#include "core.h"

#include <KServiceTypeTrader>
#include <QFile>
#include <KParts/Factory>
#include <typeinfo>
#include "partcontroller.h"
#include <QSplitter>

using namespace KDevelop;

IDocument* PatchDocumentFactory::create(const KUrl& url, ICore* core)
{
    return new PatchDocument(url, core);
}

PatchDocument::PatchDocument(const KUrl& url, ICore* core)
    : PartDocument(url, core, "Kompare/ViewPart")
{}

void PatchDocument::setDiff(const QString& from, const KUrl& to)
{
    QMap<QWidget*, KParts::Part*> parts=PartDocument::partForView();
    foreach(KParts::Part* part, parts)
    {       
        KompareInterface* iface=qobject_cast<KompareInterface*>(part);
        Q_ASSERT(iface);
        iface->compareFileString(to, from);
    }
}

QWidget* createNavPart(QWidget* parent, KParts::Part* part)
{
    KService::List offers = KServiceTypeTrader::self()->query( "KParts/ReadOnlyPart",
                                                                "'Kompare/NavigationPart' in ServiceTypes" );
    KService::Ptr ptr=offers.first();
    KParts::Factory *factory = static_cast<KParts::Factory*>    (
                        KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) ) );
    KParts::Part* navPart=factory->createPart(parent, parent,"KompareNavTreePart",
                                              QStringList("KParts::ReadOnlyPart" ));
    
    QObject::connect(part, SIGNAL(modelsChanged(const Diff2::DiffModelList*)),
                navPart, SLOT(slotModelsChanged(const Diff2::DiffModelList*)) );

    QObject::connect(part, SIGNAL(kompareInfo(Kompare::Info*)),
                navPart, SLOT(slotKompareInfo(Kompare::Info*)) );

    QObject::connect(navPart, SIGNAL(selectionChanged(const Diff2::DiffModel*,const Diff2::Difference*)),
                part, SIGNAL(selectionChanged(const Diff2::DiffModel*,const Diff2::Difference*)) );
    QObject::connect(part, SIGNAL(setSelection(const Diff2::DiffModel*,const Diff2::Difference*)),
                navPart, SLOT(slotSetSelection(const Diff2::DiffModel*,const Diff2::Difference*)) );

    QObject::connect(navPart, SIGNAL(selectionChanged(const Diff2::Difference*)),
                part, SIGNAL(selectionChanged(const Diff2::Difference*)) );
    QObject::connect(part, SIGNAL(setSelection(const Diff2::Difference*)),
                navPart, SLOT(slotSetSelection(const Diff2::Difference*)) );
                
    QObject::connect( part, SIGNAL(applyDifference(bool)),
        navPart, SLOT(slotApplyDifference(bool)) );
    QObject::connect( part, SIGNAL(applyAllDifferences(bool)),
        navPart, SLOT(slotApplyAllDifferences(bool)) );
    QObject::connect( part, SIGNAL(applyDifference(const Diff2::Difference*,bool)),
        navPart, SLOT(slotApplyDifference(const Diff2::Difference*,bool)) );
    
    return navPart->widget();
}

QWidget* PatchDocument::createViewWidget(QWidget* parent)
{
    //Overridden so that we don't get openUrl triggered by createPart
    KParts::Part *part = Core::self()->partControllerInternal()->createPart("text/x-patch", "Kompare/ViewPart");
    if( part )
    {
        QSplitter* split = new QSplitter(parent);
        QWidget* nav=createNavPart(split, part);
        
        if(url().scheme()!="kdevvcs") {
            KParts::ReadOnlyPart *ropart=dynamic_cast<KParts::ReadOnlyPart *>(part);
            ropart->openUrl(url());
        }
        
        Core::self()->partController()->addPart(part);
        
        nav->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        split->setOrientation(Qt::Vertical);
        split->addWidget(nav);
        split->addWidget(part->widget());
        addPartForView(split, part);
        
        return split;
    }
    return 0;
}
