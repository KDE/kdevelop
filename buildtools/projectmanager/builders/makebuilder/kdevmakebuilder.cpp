
#include "kdevmakebuilder.h"
#include <kdebug.h>

KDevMakeBuilder::KDevMakeBuilder(QObject *parent, const char *name)
    : KDevProjectBuilder(parent, name)
{
}

KDevMakeBuilder::~KDevMakeBuilder()
{
}

bool KDevMakeBuilder::isExecutable(ProjectItemDom dom) const
{
    Q_UNUSED(dom);
    return false;
}

ProjectItemDom KDevMakeBuilder::defaultExecutable() const
{
    return m_defaultExecutable;
}

void KDevMakeBuilder::setDefaultExecutable(ProjectItemDom dom)
{
    m_defaultExecutable = dom;
}

bool KDevMakeBuilder::build(ProjectItemDom dom)
{
    // ### we need the KDevPartController here!!
    
    if (ProjectFolderDom folder = dom->toFolder()) {
        // ### compile the folder
    } else if (ProjectTargetDom target = dom->toTarget()) {
        // ### compile the target
    } else if (ProjectFileDom file = dom->toFile()) {
        // ### compile the file
    }
    
    return false;
}

bool KDevMakeBuilder::clean(ProjectItemDom dom)
{
    Q_UNUSED(dom);
    return false;
}

bool KDevMakeBuilder::execute(ProjectItemDom dom)
{
    Q_UNUSED(dom);
    return false;
}

