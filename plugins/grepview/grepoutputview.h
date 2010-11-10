#ifndef GREPOUTPUTVIEW_H
#define GREPOUTPUTVIEW_H

#include <qwidget.h>
#include <qtreeview.h>

#include <interfaces/iuicontroller.h>

#include "ui_grepoutputview.h"

class GrepViewPlugin;
class GrepJob;
class GrepOutputModel;
class GrepOutputDelegate;

class GrepOutputViewFactory: public KDevelop::IToolViewFactory
{
public:
  GrepOutputViewFactory(GrepJob *job);
  virtual QWidget* create(QWidget *parent = 0);
  virtual Qt::DockWidgetArea defaultPosition();
  virtual QString id() const;

private:
  GrepJob *m_job;
};

class GrepOutputView : public QWidget, Ui::GrepOutputView
{
  Q_OBJECT

public:
    GrepOutputView(QWidget* parent, GrepJob* job);

    GrepOutputModel* model();
    void setMessage(const QString& msg);

private:
    GrepJob* m_job;
    GrepOutputModel* m_model;
};

#endif // GREPOUTPUTVIEW_H
