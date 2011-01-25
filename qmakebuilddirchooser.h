#ifndef QMAKEBUILDDIRCHOOSER_H
#define QMAKEBUILDDIRCHOOSER_H

#include <KDialog>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/abstractfilemanagerplugin.h>
#include <interfaces/iproject.h>
#include "ui_qmakebuilddirchooser.h"

namespace Ui 
{
    class QMakeBuildDirChooser;
}

class QMakeBuildDirChooser : public KDialog, private Ui::QMakeBuildDirChooser
{
    Q_OBJECT
    
public:
    explicit QMakeBuildDirChooser(KDevelop::IProject* project, QWidget *parent = 0);
    virtual ~QMakeBuildDirChooser();

private:
    bool isValid();
    
    KDevelop::IProject* m_project;

protected slots:
    virtual void slotButtonClicked(int button);
    
private slots:
    void validate();
    void saveConfig();
};

#endif
