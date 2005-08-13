%{H_TEMPLATE}

#ifndef _%{APPNAMEUC}PREF_H_
#define _%{APPNAMEUC}PREF_H_

#include <kdialogbase.h>
#include <qframe.h>

class %{APPNAME}PrefPageOne;
class %{APPNAME}PrefPageTwo;

class %{APPNAME}Preferences : public KDialogBase
{
    Q_OBJECT
public:
    %{APPNAME}Preferences();

private:
    %{APPNAME}PrefPageOne *m_pageOne;
    %{APPNAME}PrefPageTwo *m_pageTwo;
};

class %{APPNAME}PrefPageOne : public QFrame
{
    Q_OBJECT
public:
    %{APPNAME}PrefPageOne(QWidget *parent = 0);
};

class %{APPNAME}PrefPageTwo : public QFrame
{
    Q_OBJECT
public:
    %{APPNAME}PrefPageTwo(QWidget *parent = 0);
};

#endif // _%{APPNAMEUC}PREF_H_
