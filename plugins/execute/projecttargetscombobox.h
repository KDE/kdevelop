#ifndef PROJECTTARGETSCOMBOBOX_H
#define PROJECTTARGETSCOMBOBOX_H

#include <QtGui/QComboBox>

namespace KDevelop {
class ProjectFolderItem;
}

class ProjectTargetsComboBox : public QComboBox
{
    Q_OBJECT
    public:
        ProjectTargetsComboBox(QWidget* parent = 0);
        
        void setBaseItem(KDevelop::ProjectFolderItem* item);
        void setCurrentItemPath(const QStringList& str);
        
        QStringList currentItemPath() const;
        
};

#endif // PROJECTTARGETSCOMBOBOX_H
