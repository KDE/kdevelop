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
    SearchDialog(QStrList *searchFor, QStrList *replaceWith,
      int flags, QWidget *parent, const char *name = 0L);
    void setSearchText( const QString &searchstr );
    const char *getSearchFor();
    const char *getReplaceWith();
    int getFlags();
  protected slots:
    void okSlot();

  protected:
    QComboBox *search;
    QComboBox *replace;

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
    SettingsDialog(int flags, int wrapAt, int tabWidth, int indentLength, int
undoSteps,       QWidget *parent, const char *name = 0);
    int getFlags();
    int getWrapAt();
    int getTabWidth();
    int getIndentLength();
    int getUndoSteps();
  protected:
    QCheckBox *eopt1;
    QCheckBox *eopt2;
    QCheckBox *eopt3;
    QCheckBox *eopt4;
    QCheckBox *eopt5;
    QCheckBox *eopt6;
    QCheckBox *eopt7;
    QCheckBox *iopt1;
    QCheckBox *iopt2;
    QCheckBox *iopt3;
    QCheckBox *iopt4;
    QCheckBox *iopt5;
    QCheckBox *sopt1;
    QCheckBox *sopt2;
    QCheckBox *sopt3;
    QCheckBox *sopt4;
    QCheckBox *sopt5;
    QCheckBox *sopt6;
    QLineEdit *e1;
    QLineEdit *e2;
    QLineEdit *e3;
    QLineEdit *e4;
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

