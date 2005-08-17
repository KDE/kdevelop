#ifndef MAINVIEWIMP_H
#define MAINVIEWIMP_H

#include "mainview.h"

class MainViewImp : public MainView
{
  Q_OBJECT

public:
  MainViewImp(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
  ~MainViewImp();

public slots:
    virtual void slotHelloButtonClicked();
};

#endif

