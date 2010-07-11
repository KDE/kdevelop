#include "projecttargetscombobox.h"
#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <util/kdevstringhandler.h>
#include <KIcon>

using namespace KDevelop;

ProjectTargetsComboBox::ProjectTargetsComboBox(QWidget* parent)
    : QComboBox(parent)
{

}
    
class ExecutablePathsVisitor
    : public ProjectVisitor
{
    public:
        virtual void visit(ProjectExecutableTargetItem* eit) {
            m_paths += KDevelop::joinWithEscaping(eit->model()->pathFromIndex(eit->index()), '/', '\\');
        }
        virtual void visit(IProject* p) { ProjectVisitor::visit(p); }
        virtual void visit(ProjectBuildFolderItem* it) { ProjectVisitor::visit(it); }
        virtual void visit(ProjectFolderItem* it) { ProjectVisitor::visit(it); }
        virtual void visit(ProjectFileItem* it) { ProjectVisitor::visit(it); }
        virtual void visit(ProjectLibraryTargetItem* it) { ProjectVisitor::visit(it); }
        
        QStringList paths() const { return m_paths; }
        
    private:
        QStringList m_paths;
};


void ProjectTargetsComboBox::setBaseItem(ProjectFolderItem* item)
{
    clear();
    
    ExecutablePathsVisitor walker;
    walker.visit(item);
    
    foreach(const QString& item, walker.paths())
        addItem(KIcon("system-run"), item);
}

QStringList ProjectTargetsComboBox::currentItemPath() const
{
    return KDevelop::splitWithEscaping(currentText(), '/', '\\');
}

void ProjectTargetsComboBox::setCurrentItemPath(const QStringList& str)
{
    setCurrentIndex(findText(KDevelop::joinWithEscaping(str, '/', '\\')));
}
