/****************************************************************************
** Form interface generated from reading ui file './runoptionswidgetbase.ui'
**
** Created: Fri Apr 12 20:33:02 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef RUN_OPTIONS_WIDGET_H
#define RUN_OPTIONS_WIDGET_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QLabel;
class QLineEdit;
class QListView;
class QListViewItem;
class QPushButton;

class RunOptionsWidgetBase : public QWidget
{ 
    Q_OBJECT

public:
    RunOptionsWidgetBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~RunOptionsWidgetBase();

    QLineEdit* mainprogram_edit;
    QLabel* progargs_label_3;
    QLabel* mainprogram_label_3;
    QLabel* envvars_label;
    QLineEdit* progargs_edit;
    QCheckBox* startinterminal_box;
    QPushButton* addvar_button;
    QPushButton* removevar_button;
    QListView* listview;


protected slots:
    virtual void addVarClicked();
    virtual void removeVarClicked();

protected:
    QGridLayout* run_options_widgetLayout;
    QVBoxLayout* Layout8_3;
};

#endif // RUN_OPTIONS_WIDGET_H
