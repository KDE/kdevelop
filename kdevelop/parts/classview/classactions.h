#ifndef _CLASSACTION_H_
#define _CLASSACTION_H_

#include <kaction.h>

class CClassStore;


class ClassListAction : public KSelectAction
{
public:
    ClassListAction(const QString &text, int accel,
                    const QObject *receiver, const char *methodname,
                    QObject *parent, const char *name);
    void setClassStore(CClassStore *store);
    void setCurrentItem(const QString &item);
    void refresh();
    
private:
    CClassStore *m_store;
};


class MethodListAction : public KSelectAction
{
public:
    MethodListAction(const QString &text, int accel,
                     const QObject *receiver, const char *methodname,
                     QObject *parent, const char *name);
    void setClassStore(CClassStore *store);
    void refresh(const QString &className);

private:
    CClassStore *m_store;
};


class DelayedPopupAction : public KAction
{
    Q_OBJECT

public:
    DelayedPopupAction( const QString &text, const QString &pix, int accel,
                            QObject *receiver, const char *methname,
                            QObject *parent, const char* name );
    ~DelayedPopupAction();
    
    virtual int plug(QWidget *widget, int index=-1);
    virtual void unplug(QWidget *widget);

    QPopupMenu *popupMenu();

private:
    QPopupMenu *m_popup;
};

#endif
