// Dialogs

#ifndef _KWDIALOG_H_
#define _KWDIALOG_H_


class QCheckBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class KColorButton;
class KIntNumInput;

#include <kdialogbase.h>
#include "kwview.h"

class SearchDialog : public KDialogBase
{
    Q_OBJECT

  public:

    SearchDialog( QWidget *parent, QStringList &searchFor, QStringList &replaceWith, int flags );
    QString getSearchFor();
    QString getReplaceWith();
    int getFlags();

  protected slots:

    void slotOk();

  protected:

    QComboBox *m_search;
    QComboBox *m_replace;
    QCheckBox *m_opt1;
    QCheckBox *m_opt2;
    QCheckBox *m_opt3;
    QCheckBox *m_optRegExp;
    QCheckBox *m_opt4;
    QCheckBox *m_opt5;
    QCheckBox *m_opt6;
};

class ReplacePrompt : public KDialogBase
{
    Q_OBJECT

  public:

    ReplacePrompt(QWidget *parent);

  signals:

    void clicked();

  protected slots:

    void slotUser1( void ); // All
    void slotUser2( void ); // No
    void slotUser3( void ); // Yes
    virtual void done(int);

  protected:

    void closeEvent(QCloseEvent *);
};

class GotoLineDialog : public KDialogBase
{
    Q_OBJECT

  public:

    GotoLineDialog(QWidget *parent, int line, int max);
    int getLine();

  protected:

    KIntNumInput *e1;
    QPushButton *btnOK;
};

class IndentConfigTab : public QWidget
{
    Q_OBJECT

  public:

    IndentConfigTab(QWidget *parent, KWrite *);
    void getData(KWrite *);

  protected:

    static const int numFlags = 6;
    static const int flags[numFlags];
    QCheckBox *opt[numFlags];
};

class SelectConfigTab : public QWidget
{
    Q_OBJECT

 public:

    SelectConfigTab(QWidget *parent, KWrite *);
    void getData(KWrite *);

  protected:

    static const int numFlags = 6;
    static const int flags[numFlags];
    QCheckBox *opt[numFlags];
};

class EditConfigTab : public QWidget
{
    Q_OBJECT

  public:

    EditConfigTab(QWidget *parent, KWrite *);
    void getData(KWrite *);

  protected:

    static const int numFlags = 9;
    static const int flags[numFlags];
    QCheckBox *opt[numFlags];
    KIntNumInput *e1;
    KIntNumInput *e2;
    KIntNumInput *e3;
};

/*
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
    QCheckBox *opt13;
    KIntLineEdit *e1;
    KIntLineEdit *e2;
    KIntLineEdit *e3;
};
*/

class ColorConfig : public QWidget
{
  Q_OBJECT

public:

  ColorConfig( QWidget *parent = 0, char *name = 0 );
  ~ColorConfig();

  void setColors( QColor * );
  void getColors( QColor * );

private:

  KColorButton *m_back;
  KColorButton *m_textBack;
  KColorButton *m_selected;
  KColorButton *m_found;
  KColorButton *m_selFound;
};


#endif //_KWDIALOG_H_
