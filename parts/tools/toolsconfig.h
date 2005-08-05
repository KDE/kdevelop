#ifndef __TOOLS_CONFIG_H__
#define __TOOLS_CONFIG_H__


#include <qwidget.h>
#include <qptrlist.h>
#include <qpixmap.h>


class QListBox;
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
    QListBox *_list;
    QPushButton *_toList, *_toTree;

    QPtrList<Entry> _entries;

  };


#endif
