#ifndef __K_TABWIDGET_H__
#define __K_TABWIDGET_H__

#include <qtabwidget.h>
#include <qtabbar.h>
#include <qevent.h>
class QPopupMenu;

class KTabBar : public QTabBar
{
  Q_OBJECT

public:
  KTabBar(QWidget *parent=0, const char *name=0);
  ~KTabBar();
signals:
  void closeWindow(const QWidget *);
  void closeOthers(QWidget *);
protected slots:
  virtual void mousePressEvent(QMouseEvent *e);  
  void closeWindowSlot();
  void closeOthersSlot();
private:
  QPopupMenu *m_pPopupMenu;
  QWidget *m_pPage;
};

class KTabWidget : public QTabWidget
{
  Q_OBJECT
  
public:
  KTabWidget(QWidget *parent=0, const char *name=0);
  ~KTabWidget();
signals:
  void closeWindow(const QWidget *);
  void closeOthers(QWidget *);
private:
  KTabBar *m_pTabBar;
};

#endif
