#ifndef GREPOUTPUTVIEW_H
#define GREPOUTPUTVIEW_H

#include "qwidget.h"
#include "qtreeview.h"

#include "ui_grepoutputview.h"

class GrepViewPlugin;
class GrepJob;
class GrepOutputModel;
class GrepOutputDelegate;

class GrepOutputView : public QWidget, Ui::GrepOutputView
{
  Q_OBJECT

public:
    GrepOutputView(QWidget* parent, GrepJob* job);
	
    void setModel(GrepOutputModel*);
    void setDelegate(GrepOutputDelegate*);

private:
    GrepJob* m_job;

};

#endif // GREPOUTPUTVIEW_H
