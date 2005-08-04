#ifndef __VALGRIND_WIDGET_H__
#define __VALGRIND_WIDGET_H__

#include <qwidget.h>
#include <qstring.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3PopupMenu>

#include "valgrinditem.h"

class ValgrindPart;
class KListView;
class Q3ListViewItem;
class Q3PopupMenu;

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
  void executed( Q3ListViewItem* item );
  void slotContextMenu( KListView* l, Q3ListViewItem* i, const QPoint& p );
  void expandAll();
  void collapseAll();
  void aboutToShowPopup();
    
private:
  KListView* lv;
  int msgNumber;
  ValgrindPart* _part;
  Q3PopupMenu* popup;
};


#endif
