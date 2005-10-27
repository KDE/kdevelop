%{H_TEMPLATE}

#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <QMainWindow>
#include <QCloseEvent>

class QAction;
class QMenu;
class QTextEdit;

class %{APPNAME}:public QMainWindow
{
      Q_OBJECT

public:
      %{APPNAME}();
      ~%{APPNAME}();

protected:
      void closeEvent(QCloseEvent *event);

private slots:
      void newFile();
      void open();
      bool save();
      bool saveAs();
      void about();
      void documentWasModified();

private:
      void createActions();
      void createMenus();
      void createToolBars();
      void createStatusBar();
      void readSettings();
      void writeSettings();
      bool maybeSave();
      void loadFile(const QString &fileName);
      bool saveFile(const QString &fileName);
      void setCurrentFile(const QString &fileName);
      QString strippedName(const QString &fullFileName);

      QTextEdit *textEdit;
      QString curFile;

      QMenu *fileMenu;
      QMenu *editMenu;
      QMenu *helpMenu;
      QToolBar *fileToolBar;
      QToolBar *editToolBar;
      QAction *newAct;
      QAction *openAct;
      QAction *saveAct;
      QAction *saveAsAct;
      QAction *exitAct;
      QAction *cutAct;
      QAction *copyAct;
      QAction *pasteAct;
      QAction *aboutAct;
      QAction *aboutQtAct;
};

#endif
