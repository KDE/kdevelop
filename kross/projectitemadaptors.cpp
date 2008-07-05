#include "projectitemadaptors.h"

using namespace KDevelop;

ProjectBaseItemAdaptor* ProjectBaseItemAdaptor::createAdaptor(QObject* parent, ProjectBaseItem* item)
{
    ProjectBaseItemAdaptor* adaptor=0;
    switch(item->type())
    {
        case ProjectBaseItem::BuildFolder:
        case ProjectBaseItem::Folder:
            adaptor=new ProjectFolderItemAdaptor(parent, (ProjectFolderItem*) item);
            break;
        case ProjectBaseItem::File:
            adaptor=new ProjectFileItemAdaptor(parent, (ProjectFileItem*) item);
            break;
        case ProjectBaseItem::Target:
            adaptor=new ProjectTargetItemAdaptor(parent, (ProjectTargetItem*) item);
            break;
        default:
            qFatal("unknown project model type");
            break;
    }
    Q_ASSERT(adaptor);
    return adaptor;
}


#include "projectitemadaptors.moc"

