//Dialogs

#ifndef _KWDIALOG_H_
#define _KWDIALOG_H_

#include <qdialog.h>
#include <qlined.h>
#include <qchkbox.h>
#include <qradiobt.h>
#include <qlistbox.h>

#include <kcolorbtn.h>

#include "kwview.h"

class SearchDialog : public QDialog {
    Q_OBJECT
  public:
    SearchDialog(const char *searchFor, const char *replaceWith,
      int flags, QWidget *parent, const char *name = 0);
    const char *getSearchFor();
    const char *getReplaceWith();
    int getFlags();
//  signals:
//    void search();
//    void doneSearch();
  protected slots:
    void okSlot();

  protected:
    QLineEdit *search;
    QLineEdit *replace;

    QCheckBox *opt1;
    QCheckBox *opt2;
    QCheckBox *opt3;
    QCheckBox *opt4;
    QCheckBox *opt5;
    QCheckBox *opt6;
};

class ReplacePrompt : public QDialog {
    Q_OBJECT
  public:
    ReplacePrompt(QWidget *parent, const char *name = 0);
  signals:
    void clicked();
  protected slots:
    void no();
    void all();
    virtual void done(int);
  protected:
    void closeEvent(QCloseEvent *);
};

class GotoLineDialog : public QDialog {
    Q_OBJECT
  public:
    GotoLineDialog(int line, QWidget *parent, const char *name = 0);
    int getLine();
  protected:
    QLineEdit *e1;
};

class SettingsDialog : public QDialog {
    Q_OBJECT
  public:
    SettingsDialog(int flags, int wrapAt, int tabWidth, int undoSteps,
      QWidget *parent, const char *name = 0);
    int getFlags();
    int getWrapAt();
    int getTabWidth();
    int getUndoSteps();
  protected:
    QCheckBox *opt1;
    QCheckBox *opt2;
    QCheckBox *opt3;
    QCheckBox *opt4;
    QCheckBox *opt5;
    QCheckBox *opt6;
    QCheckBox *opt7;
    QCheckBox *opt8;
    QCheckBox *opt9;
    QCheckBox *opt10;
    QCheckBox *opt11;
    QCheckBox *opt12;
    QLineEdit *e1;
    QLineEdit *e2;
    QLineEdit *e3;
};

class ColorDialog : public QDialog {
    Q_OBJECT
  public:
    ColorDialog(QColor *, QWidget *parent, const char *name = 0);
    void getColors(QColor *);
  protected:
    KColorButton *back;
    KColorButton *textBack;
    KColorButton *selected;
    KColorButton *found;
    KColorButton *selFound;
};


#endif //_KWDIALOG_H_
