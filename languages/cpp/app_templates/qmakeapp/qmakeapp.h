%{H_TEMPLATE}

#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <qmainwindow.h>

class QTextEdit;

class %{APPNAME}: public QMainWindow
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
    QTextEdit *e;
    QString filename;
};


#endif
