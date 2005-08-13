#ifndef __VALGRIND_WIDGET_H__
#define __VALGRIND_WIDGET_H__

#include <qwidget.h>
#include <qstring.h>
#include <qstringlist.h>

#include "valgrinditem.h"

class ValgrindPart;
class KListView;
class QListViewItem;
class QPopupMenu;

class ValgrindWidget : public QWidget
{
  Q_OBJECT
    
public:
  ValgrindWidget( ValgrindPart *part );
  ~ValgrindWidget();
  
  void addMessage( const ValgrindItem& vi );
  void clear();

signals:
  void jumpToFile( const QString& url, int line );
  
private slots:
  void executed( QListViewItem* item );
  void slotContextMenu( KListView* l, QListViewItem* i, const QPoint& p );
  void expandAll();
  void collapseAll();
  void aboutToShowPopup();
    
private:
  KListView* lv;
  int msgNumber;
  ValgrindPart* _part;
  QPopupMenu* popup;
};


#endif
