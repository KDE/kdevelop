#ifndef __TOOLS_CONFIG_H__
#define __TOOLS_CONFIG_H__


#include <qwidget.h>
#if (QT_VERSION >= 300)
#include <qptrlist.h>
#else
#include <qlist.h>
#define QPtrList QList
#define QPtrListIterator QListIterator
#endif
#include <qpixmap.h>


class TreeView;
class QListBox;
class QPushButton;


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

    TreeView *_tree;
    QListBox *_list;
    QPushButton *_toList, *_toTree;

    QPtrList<Entry> _entries;

  };


#endif
