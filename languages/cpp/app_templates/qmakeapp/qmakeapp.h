//Added by qt3to4:
#include <QCloseEvent>
%{H_TEMPLATE}

#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <q3mainwindow.h>

class Q3TextEdit;

class %{APPNAME}: public Q3MainWindow
{
    Q_OBJECT

public:
    %{APPNAME}();
    ~%{APPNAME}();

protected:
    void closeEvent( QCloseEvent* );

private slots:
    void newDoc();
    void choose();
    void load( const QString &fileName );
    void save();
    void saveAs();
    void print();

    void about();
    void aboutQt();

private:
    QPrinter *printer;
    Q3TextEdit *e;
    QString filename;
};


#endif
