#ifndef _PROJECTOPTIONSDLG_H_
#define _PROJECTOPTIONSDLG_H_

#include <kdialogbase.h>

class KDevCompilerOptions;

class ProjectOptionsDialog : public KDialogBase
{
    Q_OBJECT
    
public:
    ProjectOptionsDialog( QWidget *parent=0, const char *name=0 );
    ~ProjectOptionsDialog();

protected:
    void accept();
    
private:
    KDevCompilerOptions *createCompilerOptions(const QString &lang, QWidget *box);
    KDevCompilerOptions *coptdlg;
    KDevCompilerOptions *cppoptdlg;
};

#endif
