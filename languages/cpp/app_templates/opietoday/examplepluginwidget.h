#ifndef %{APPNAME}_PLUGIN_WIDGET_H
#define %{APPNAME}_PLUGIN_WIDGET_H

#include <qlayout.h>

#include <opie/oclickablelabel.h>

class %{APPNAME}Widget : public QWidget {

    Q_OBJECT

public:
    %{APPNAME}Widget( QWidget *parent,  const char *name );
    ~%{APPNAME}Widget();
    
    void refresh();

private slots:
   void slotClicked();

private:
    OClickableLabel* m_exampleLabel;
    QHBoxLayout* m_layout;
    void readConfig();
    void getInfo();
};

#endif
