#ifndef _FLAGBOXES_H_
#define _FLAGBOXES_H_

#include <qlistview.h>
#include <qcheckbox.h>
#include <qlist.h>

class FlagListBox;
class FlagListToolTip;
class FlagCheckBoxController;


class FlagListItem : public QCheckListItem
{
public:
    FlagListItem(FlagListBox *parent, 
                 const QString &flagstr, const QString &description);
    ~FlagListItem()
        {}
private:
    QString flag;
    QString desc;
    friend class FlagListToolTip;
    friend class FlagListBox;
};


class FlagListBox : public QListView
{
    Q_OBJECT
public:
    FlagListBox( QWidget *parent=0, const char *name=0 );
    ~FlagListBox()
        {}

    void readFlags(QStringList *list);
    void writeFlags(QStringList *list);
};


class FlagCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    FlagCheckBox(QWidget *parent, FlagCheckBoxController *controller,
                 const QString &flagstr, const QString &description);
    ~FlagCheckBox()
        {}

private:
    QString flag;
    friend class FlagCheckBoxController;
};


class FlagCheckBoxController
{
public:
    FlagCheckBoxController();
    ~FlagCheckBoxController()
        {}

    void readFlags(QStringList *list);
    void writeFlags(QStringList *list);

private:
    void addCheckBox(FlagCheckBox *item);
    QList<FlagCheckBox> cblist;
    friend class FlagCheckBox;
};

#endif
