#ifndef __TOOLS_CONFIG_H__
#define __TOOLS_CONFIG_H__


#include <QWidget>
#include <q3ptrlist.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QShowEvent>


class Q3ListBox;
class QPushButton;
class KDevApplicationTree;

class Entry 
{
public:

  Entry() {};

  QString name;
  QString desktopFile;
  QPixmap icon;

};


class ToolsConfig : public QWidget
  {
    Q_OBJECT

public:

    ToolsConfig(QWidget *parent = 0, const char *name = 0);


public slots:
   
    void accept();


protected:

    void showEvent(QShowEvent *);

    void fill();
    void add(const QString &desktopFile);
    void remove(const QString &desktopFile);


private slots:

    void checkButtons();

    void toList();
    void toTree();

    void updateList();


private:

    KDevApplicationTree *_tree;
    Q3ListBox *_list;
    QPushButton *_toList, *_toTree;

    Q3PtrList<Entry> _entries;

  };


#endif
