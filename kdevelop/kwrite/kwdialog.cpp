//Search dialog

#include <stdlib.h>
#include <stdio.h>

#include <qobjectlist.h>
#include <qlabel.h>
#include <qbttngrp.h>
#include <qpushbt.h>
#include <qobjcoll.h>
#include <qlayout.h>

#include <kapp.h>

#include "kwdialog.h"
#include "kwdoc.h"
#include <klocale.h>
#include <knumvalidator.h>

SearchDialog::SearchDialog(QStrList *searchFor, QStrList *replaceWith,
  int flags, QWidget *parent, const char *name)
  : QDialog(parent,name,true) {
  
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 8 );

  QLabel *label1, *label2;
  QPushButton *button1, *button2;;
  
  search = new QComboBox(true, this);
  search->insertStrList(searchFor);
  search->setFocus();
  QLineEdit* pLE = search->lineEdit();
  if (pLE) {
    pLE->selectAll();
  }
//  search->dumpObjectTree();
  search->setFixedHeight( search->sizeHint().height() );
  search->setMinimumWidth( search->sizeHint().width() );
//  search->selectAll();
  label1 = new QLabel(search,i18n("&Text To Find:"),this);
  label1->setFixedSize( label1->sizeHint() );
  mainLayout->addWidget( label1, 0, AlignLeft );
  mainLayout->addWidget( search, 0, AlignLeft );

  if (flags & sfReplace) {
    replace = new QComboBox(true, this);
    replace->insertStrList(replaceWith);
    replace->setFixedHeight( replace->sizeHint().height() );
    replace->setMinimumWidth( replace->sizeHint().width() );
    label2 = new QLabel(replace,i18n("&Replace With:"),this);
    label2->setFixedSize( label2->sizeHint() );
    mainLayout->addWidget( label2, 0, AlignLeft );
    mainLayout->addWidget( replace, 0, AlignLeft );
  } else replace = 0;

  QGroupBox *group = new QGroupBox(i18n("Options"),this);
  mainLayout->addWidget( group );

  QHBoxLayout *vbl1 = new QHBoxLayout( group, 10, 4 );
  QVBoxLayout *vbl2 = new QVBoxLayout();
  vbl1->addLayout(vbl2);
  vbl2->addSpacing( 10 );
  opt1 = new QCheckBox(i18n("&Case Sensitive"),group);
  opt1->setFixedSize( opt1->sizeHint() );
  vbl2->addWidget( opt1, 0, AlignLeft );
  int max2=opt1->sizeHint().width();
  opt2 = new QCheckBox(i18n("&Whole Words Only"),group);
  opt2->setFixedSize( opt2->sizeHint() );
  vbl2->addWidget( opt2, 0, AlignLeft );
  if(opt2->sizeHint().width()>max2) max2 = opt2->sizeHint().width();
  opt3 = new QCheckBox(i18n("&From Cursor"),group);
  opt3->setFixedSize( opt3->sizeHint() );
  vbl2->addWidget( opt3, 0, AlignLeft );
  if(opt3->sizeHint().width()>max2) max2 = opt3->sizeHint().width();


  QVBoxLayout *vbl3 = new QVBoxLayout();
  vbl1->addLayout(vbl3);
  vbl3->addSpacing( 10 );
  opt4 = new QCheckBox(i18n("Find &Backwards"),group);
  opt4->setFixedSize( opt4->sizeHint() );
  vbl3->addWidget( opt4, 0, AlignLeft );
  int max3=opt4->sizeHint().width();
  opt5 = new QCheckBox(i18n("&Selected Text"),group);
  opt5->setFixedSize( opt5->sizeHint() );
  vbl3->addWidget( opt5, 0, AlignLeft );
  if(opt5->sizeHint().width()>max3) max3 = opt5->sizeHint().width();

  opt1->setChecked(flags & sfCaseSensitive);
  opt2->setChecked(flags & sfWholeWords);
  opt3->setChecked(flags & sfFromCursor);
  opt4->setChecked(flags & sfBackward);
  opt5->setChecked(flags & sfSelected);

  if (replace) {
    opt6 = new QCheckBox(i18n("&Prompt On Replace"),group);
    opt6->setChecked(flags & sfPrompt);
    opt6->setFixedSize( opt6->sizeHint() );
    vbl3->addWidget( opt6, 0, AlignLeft );
    if(opt6->sizeHint().width()>max3) max3 = opt6->sizeHint().width();
  }

  group->setMinimumHeight(10+10+10+2*4+3*(opt1->sizeHint().height()));
  group->setMinimumWidth(max2+max3+10+10+10+10+4+5);

  search->setMinimumWidth(group->width());
  if( replace )
    replace->setMinimumWidth(group->width());

  mainLayout->addSpacing(10);
  mainLayout->addStretch( 1 );

  QHBoxLayout *hbl2 = new QHBoxLayout();
  mainLayout->addLayout( hbl2 );

  hbl2->addStretch( 1 );
  button1 = new QPushButton(i18n("&OK"),this);
  button1->setFixedSize( button1->sizeHint() );
  button1->setDefault(true);
  hbl2->addWidget( button1 );
  connect(button1,SIGNAL(clicked()),this,SLOT(okSlot()));

  button2 = new QPushButton(i18n("Cancel"),this);
  button2->setFixedSize( button2->sizeHint() );
  hbl2->addWidget( button2 );
  connect(button2,SIGNAL(clicked()),this,SLOT(reject()));//SIGNAL(doneSearch()));

  mainLayout->activate();
  mainLayout->freeze();
//  resize(minimumSize());
  
  search->setFocus();
}

void SearchDialog::setSearchText( const QString &searchstr )
{
  search->insertItem( searchstr, 0 );
  search->setCurrentItem( 0 );
  QLineEdit* pLE = search->lineEdit();
  if (pLE) {
    pLE->selectAll();
  }
}

const char *SearchDialog::getSearchFor() {
  return search->currentText();
}

const char *SearchDialog::getReplaceWith() {
  return replace->currentText();
}

int SearchDialog::getFlags() {
  int flags = 0;

  if (opt1->isChecked()) flags |= sfCaseSensitive;
  if (opt2->isChecked()) flags |= sfWholeWords;
  if (opt3->isChecked()) flags |= sfFromCursor;
  if (opt4->isChecked()) flags |= sfBackward;
  if (opt5->isChecked()) flags |= sfSelected;
  if (replace) {
    if (opt6->isChecked()) flags |= sfPrompt;
    flags |= sfReplace;
  }

  return flags;
}

void SearchDialog::okSlot() {
  const char *text;

  text = search->currentText();
  if (text && *text) accept();//emit search();
}

ReplacePrompt::ReplacePrompt(QWidget */*parent*/, const char *name)
  : QDialog(0L,name,false) {

  QVBoxLayout *mainLayout = new QVBoxLayout(this, 10);
  
  QLabel *label;
  QPushButton *button1, *button2, *button3, *button4;

  label = new QLabel(i18n("Replace this occurence?"),this);
  label->setFixedSize( label->sizeHint() );
  mainLayout->addWidget(label, 0, AlignLeft);

  QHBoxLayout *hbl = new QHBoxLayout();
  mainLayout->addLayout(hbl);
  hbl->addStretch();
  button1 = new QPushButton(i18n("&Yes"),this);
  button1->setFixedSize( button1->sizeHint() );
  button1->setDefault(true);
  button1->setAutoDefault(true);
  hbl->addWidget(button1);
  hbl->addStretch();
  connect(button1,SIGNAL(clicked()),this,SLOT(accept()));

  button2 = new QPushButton(i18n("&No"),this);
  button2->setFixedSize( button2->sizeHint() );
  hbl->addWidget(button2);
  hbl->addStretch();
  connect(button2,SIGNAL(clicked()),this,SLOT(no()));

  button3 = new QPushButton(i18n("&All"),this);
  button3->setFixedSize( button3->sizeHint() );
  hbl->addWidget(button3);
  hbl->addStretch();
  connect(button3,SIGNAL(clicked()),this,SLOT(all()));

  button4 = new QPushButton(i18n("&Close"),this);
  button4->setFixedSize( button4->sizeHint() );
  hbl->addWidget(button4);
  hbl->addStretch();
  connect(button4,SIGNAL(clicked()),this,SLOT(reject()));

  mainLayout->activate();
  mainLayout->freeze();
//  resize(minimumSize());

//  if (parent) {
//    QWidget *w;
//    QPoint p(0,0);
//
//    w = parent;//->topLevelWidget();
//    p = w->mapToGlobal(p);
//    move(p.x() + (w->width() - width())/2,
//         p.y() + w->height() - height() - 48);
//  }

}

//void qt_leave_modal(QWidget *);

void ReplacePrompt::no() {
  done(srNo);
}

void ReplacePrompt::all() {
  done(srAll);
}


void ReplacePrompt::done(int r) {
  setResult(r);
  emit clicked();
//  qt_leave_modal(this);
//  kapp->exit_loop();
}

void ReplacePrompt::closeEvent(QCloseEvent *) {
  reject();
}

GotoLineDialog::GotoLineDialog(int line, QWidget *parent, const char *name)
  : QDialog(parent,name,true) {

  char buf[8];
  QLabel *label;
  QPushButton *button1, *button2;

  QVBoxLayout *mainLayout = new QVBoxLayout(this, 10);

  e1 = new QLineEdit(this);
  e1->setValidator( new KIntValidator( e1 ) );
  sprintf(buf,"%d",line);
  e1->setText(buf);
  e1->selectAll();
  e1->setFixedHeight( e1->sizeHint().height() );
  label = new QLabel(e1,i18n("&Goto Line:"),this);
  label->setFixedSize( label->sizeHint() );
  mainLayout->addWidget(label, 0, AlignLeft);
  mainLayout->addWidget(e1, 0, AlignLeft);

  QHBoxLayout *hbl = new QHBoxLayout();
  mainLayout->addSpacing(10);
  mainLayout->addLayout(hbl);
  hbl->addStretch(1);

  button1 = new QPushButton(i18n("&OK"),this);
  button1->setDefault(true);
  button1->setFixedSize( button1->sizeHint() );
  hbl->addWidget(button1);
  connect(button1,SIGNAL(clicked()),this,SLOT(accept()));

  button2 = new QPushButton(i18n("Cancel"),this);
  connect(button2,SIGNAL(clicked()),this,SLOT(reject()));
  button2->setFixedSize( button2->sizeHint() );
  hbl->addWidget(button2);

  e1->setMinimumWidth(minimumSize().width());

  mainLayout->activate();
  mainLayout->freeze();
//  resize(minimumSize());

  e1->setFocus();
}

int GotoLineDialog::getLine() {
  return atoi(e1->text());
}


SettingsDialog::SettingsDialog(int flags, int wrapAt, int tabWidth,
                               int indentLength, int undoSteps,
                               QWidget* parent, const char *name)
  : QDialog(parent,name,true) {
  QLabel *label1;
  QLabel *label2;
  QLabel *label3;
  QLabel *label4;
  QPushButton *button1, *button2;
  char buf[8];

  //Uncomment to restore old layout
  //#define VERTICAL_LAYOUT

  QGroupBox *g1 = new QGroupBox(i18n("Edit Options"),this);
  eopt1 = new QCheckBox(i18n("&Word Wrap"),g1);
  eopt2 = new QCheckBox(i18n("&Replace Tabs"),g1);
  eopt3 = new QCheckBox(i18n("Remove Trailing &Spaces"),g1);
  eopt4 = new QCheckBox(i18n("Wrap &Cursor"),g1);
  eopt5 = new QCheckBox(i18n("&Auto Brackets"),g1);
  eopt6 = new QCheckBox(i18n("&Highlight Brackets"),g1);
  eopt7 = new QCheckBox(i18n("Sh&ow tabs"), g1);
  // shortcuts: achorsw
  eopt1->setChecked(flags & cfWordWrap);
  eopt2->setChecked(flags & cfReplaceTabs);
  eopt3->setChecked(flags & cfRemoveSpaces);
  eopt4->setChecked(flags & cfWrapCursor);
  eopt5->setChecked(flags & cfAutoBrackets);
  eopt6->setChecked(flags & cfHighlightBrackets);
  eopt7->setChecked(flags & cfShowTabs);
  // g1->setMinimumHeight(8+8+8+6*4+9*(opt1->sizeHint().height()));

  QGroupBox *g2 = new QGroupBox(i18n("Indentation Options"),this);
  iopt1 = new QCheckBox(i18n("Enter &Indents"),g2);
  iopt2 = new QCheckBox(i18n("Ta&b Indents"),g2);
  iopt3 = new QCheckBox(i18n("Bac&kspace Unindents"),g2);
  iopt4 = new QCheckBox(i18n("Indent Brac&es"),g2);
  iopt5 = new QCheckBox(i18n("Indent Pare&ntheses"),g2); // unimplemented
  // shortcuts: beikn
  iopt1->setChecked(flags & cfAutoIndent);
  iopt2->setChecked(flags & cfTabIndent);
  iopt3->setChecked(flags & cfBackspaceIndent);
  iopt4->setChecked(flags & cfIndentBraces);
  iopt5->setChecked(flags & cfIndentParentheses);
  iopt5->hide(); // FIXME cfIndentParentheses flag unimplemented yet

  QGroupBox *g3 = new QGroupBox(i18n("Selection Options"),this);
  // This cannot be "Select options" because it doesn't seem to be correct english
  // grammar to me. Select is a verb, selection is a noun. It's an i18n nightmare
  // because of that, e.g. in Polish, it translates to "Choose options", with Choose
  // being a verb.
  sopt1 = new QCheckBox(i18n("&Persistent Selections"),g3);
  sopt2 = new QCheckBox(i18n("&Multiple Selections"),g3);
  sopt3 = new QCheckBox(i18n("&Vertical Selections"),g3);
  sopt4 = new QCheckBox(i18n("&Delete On Input"),g3);
  sopt5 = new QCheckBox(i18n("&Toggle Old"),g3);
  sopt6 = new QCheckBox(i18n("A&uto Copy"),g3);
  // shortcuts: dmptuv
  //g3->setMinimumHeight(8+8+8+6*4+6*(opt9->sizeHint().height()));
  sopt1->setChecked(flags & cfPersistent);
  sopt2->setChecked(flags & cfKeepSelection);
  sopt3->setChecked(flags & cfVerticalSelect);
  sopt4->setChecked(flags & cfDelOnInput);
  sopt5->setChecked(flags & cfXorSelect);
  sopt6->setChecked(flags & cfAutoCopy);

  #ifdef VERTICAL_LAYOUT
  #define parent this
  #else
  #define parent g1
  #endif

  e1 = new QLineEdit(parent);
  sprintf(buf,"%d",wrapAt);
  e1->setText(buf);
  e1->setValidator( new KIntValidator( e1 ) );
  label1 = new QLabel(e1,i18n("Wrap Words At:"),parent);
  int max = label1->sizeHint().width();

  #ifndef VERTICAL_LAYOUT
  #undef parent
  #define parent g2
  #endif

  e2 = new QLineEdit(parent);
  sprintf(buf,"%d",tabWidth);
  e2->setText(buf);
  e2->setValidator( new KIntValidator( e2 ) );
  label2 = new QLabel(e2,i18n("Tab Width:"),parent);
  if (label2->sizeHint().width()>max)
    max = label2->sizeHint().width();

  #ifndef VERTICAL_LAYOUT
  #undef parent
  #define parent g1
  #endif

  e3 = new QLineEdit(parent);
  sprintf(buf,"%d",undoSteps);
  e3->setText(buf);
  e3->setValidator( new KIntValidator( e3 ) );
  label3 = new QLabel(e3,i18n("Undo steps:"),parent);
  if (label3->sizeHint().width()>max)
    max = label3->sizeHint().width();

  #ifndef VERTICAL_LAYOUT
  #undef parent
  #define parent g2
  #endif

  e4 = new QLineEdit(parent);
  sprintf(buf,"%d",indentLength);
  e4->setText(buf);
  e4->setValidator( new KIntValidator( e4 ) );
  label4 = new QLabel(e4,i18n("Indentation Length"),parent);
    max = label4->sizeHint().width();

  #undef parent

  #ifdef VERTICAL_LAYOUT
  label1->setFixedSize( max, label1->sizeHint().height() );
  label2->setFixedSize( max, label2->sizeHint().height() );
  label3->setFixedSize( max, label3->sizeHint().height() );
  label4->setFixedSize( max, label4->sizeHint().height() );
  #endif

  e1->setFixedSize( max, e1->sizeHint().height() );
  e2->setFixedSize( max, e2->sizeHint().height() );
  e3->setFixedSize( max, e3->sizeHint().height() );
  e4->setFixedSize( max, e4->sizeHint().height() );

  button1 = new QPushButton(i18n("&OK"),this);
  button1->setFixedSize(button1->sizeHint());
  button1->setDefault(true);
  connect(button1,SIGNAL(clicked()),this,SLOT(accept()));

  button2 = new QPushButton(i18n("Cancel"),this);
  button2->setFixedSize(button2->sizeHint());
  connect(button2,SIGNAL(clicked()),this,SLOT(reject()));


  QVBoxLayout *mainLayout = new QVBoxLayout(this, 8, 4);

  QHBoxLayout *hbl1 = new QHBoxLayout();
  mainLayout->addLayout( hbl1 );

  #ifdef VERTICAL_LAYOUT
  QVBoxLayout *vbl4 = new QVBoxLayout();
  hbl1->addLayout( vbl4 );
  #endif

  // Edit options
  QVBoxLayout *vbl1 = new QVBoxLayout(g1, 8, 4);
  vbl1->addSpacing(8);
  vbl1->addWidget( eopt1,0,AlignLeft );
  vbl1->addWidget( eopt2,0,AlignLeft );
  vbl1->addWidget( eopt3,0,AlignLeft );
  vbl1->addWidget( eopt4,0,AlignLeft );
  vbl1->addWidget( eopt5,0,AlignLeft );
  vbl1->addWidget( eopt6,0,AlignLeft );
  vbl1->addWidget( eopt7,0,AlignLeft );
  #ifndef VERTICAL_LAYOUT
  vbl1->addStretch();
  vbl1->addSpacing(8);
  vbl1->addWidget( label1,0,AlignLeft );
  vbl1->addWidget( e1,0,AlignLeft);
  vbl1->addSpacing(8);
  vbl1->addWidget( label3,0,AlignLeft );
  vbl1->addWidget( e3,0,AlignLeft);
  #endif

  // Indentation options
  QVBoxLayout *vbl2 = new QVBoxLayout(g2, 8, 4);
  vbl2->addSpacing(8);
  vbl2->addWidget( iopt1,0,AlignLeft );
  vbl2->addWidget( iopt2,0,AlignLeft );
  vbl2->addWidget( iopt3,0,AlignLeft );
  vbl2->addWidget( iopt4,0,AlignLeft );
  vbl2->addWidget( iopt5,0,AlignLeft );
  #ifndef VERTICAL_LAYOUT
  vbl2->addStretch();
  vbl2->addSpacing(8);
  vbl2->addWidget( label2,0,AlignLeft );
  vbl2->addWidget( e2,0,AlignLeft);
  vbl2->addSpacing(8);
  vbl2->addWidget( label4,0,AlignLeft );
  vbl2->addWidget( e4,0,AlignLeft);
  #endif

  // Selection options
  QVBoxLayout *vbl3 = new QVBoxLayout(g3, 8, 4);
  vbl3->addSpacing(8);
  vbl3->addWidget( sopt1,0,AlignLeft );
  vbl3->addWidget( sopt2,0,AlignLeft );
  vbl3->addWidget( sopt3,0,AlignLeft );
  vbl3->addWidget( sopt4,0,AlignLeft );
  vbl3->addWidget( sopt5,0,AlignLeft );
  vbl3->addWidget( sopt6,0,AlignLeft );
  #ifndef VERTICAL_LAYOUT
  vbl3->addStretch();
  #endif

  #ifdef VERTICAL_LAYOUT
  vbl4->addWidget( g1 );
  vbl4->addSpacing( 10 );
  vbl4->addWidget( g2 );
  vbl4->addSpacing( 10 );
  vbl4->addWidget( g3 );

  hbl1->addSpacing( 10 );
  #else
  hbl1->addWidget( g1 );
  hbl1->addSpacing( 10 );
  hbl1->addWidget( g2 );
  hbl1->addSpacing( 10 );
  hbl1->addWidget( g3 );
  #endif

  #ifdef VERTICAL_LAYOUT
  QVBoxLayout *vbl5 = new QVBoxLayout();
  hbl1->addLayout( vbl5 );

  vbl5->addWidget( label1,0,AlignLeft );
  vbl5->addWidget( e1,0,AlignLeft );
  vbl5->addSpacing( 20 );
  vbl5->addWidget( label2,0,AlignLeft );
  vbl5->addWidget( e2,0,AlignLeft );
  vbl5->addSpacing( 20 );
  vbl5->addWidget( label3,0,AlignLeft );
  vbl5->addWidget( e3,0,AlignLeft );
  vbl5->addSpacing( 20 );
  vbl5->addWidget( label4,0,AlignLeft );
  vbl5->addWidget( e4,0,AlignLeft );
  #endif

  mainLayout->addSpacing( 10 );
  mainLayout->addStretch( 1 );

  QHBoxLayout *hbl2 = new QHBoxLayout();
  mainLayout->addLayout( hbl2 );
  hbl2->addStretch( 1 );
  hbl2->addWidget(button1);
  hbl2->addWidget(button2);

  mainLayout->activate();
  resize(minimumSize());

  #undef VERTICAL_LAYOUT

}

int SettingsDialog::getFlags() {
  int flags;

  flags = 0;

  if (eopt1->isChecked()) flags |= cfWordWrap;
  if (eopt2->isChecked()) flags |= cfReplaceTabs;
  if (eopt3->isChecked()) flags |= cfRemoveSpaces;
  if (eopt4->isChecked()) flags |= cfWrapCursor;
  if (eopt5->isChecked()) flags |= cfAutoBrackets;
  if (eopt6->isChecked()) flags |= cfHighlightBrackets;
  if (eopt7->isChecked()) flags |= cfShowTabs;

  if (iopt1->isChecked()) flags |= cfAutoIndent;
  if (iopt2->isChecked()) flags |= cfTabIndent;
  if (iopt3->isChecked()) flags |= cfBackspaceIndent;
  if (iopt4->isChecked()) flags |= cfIndentBraces;
  if (iopt5->isChecked()) flags |= cfIndentParentheses;

  if (sopt1->isChecked()) flags |= cfPersistent;
  if (sopt2->isChecked()) flags |= cfKeepSelection;
  if (sopt3->isChecked()) flags |= cfVerticalSelect;
  if (sopt4->isChecked()) flags |= cfDelOnInput;
  if (sopt5->isChecked()) flags |= cfXorSelect;
  if (sopt6->isChecked()) flags |= cfAutoCopy;
  return flags;
}

int SettingsDialog::getWrapAt() {
  return atoi(e1->text());
}

int SettingsDialog::getTabWidth() {
  return atoi(e2->text());
}

int SettingsDialog::getUndoSteps() {
  return atoi(e3->text());
}

int SettingsDialog::getIndentLength() {
  return atoi(e4->text());
}

ColorDialog::ColorDialog(QColor *colors, QWidget *parent, const char *name)
  : QDialog(parent,name,true) {
  
  QVBoxLayout *mainLayout = new QVBoxLayout(this, 10);
  
  QLabel *label;
  QPushButton *button;
  
  back = new KColorButton(colors[4],this);
  label = new QLabel(back,i18n("Background:"),this);
  label->setFixedSize( label->sizeHint() );
  back->setFixedSize( back->sizeHint() );
  mainLayout->addWidget(label, 0, AlignLeft);
  mainLayout->addWidget(back, 0, AlignLeft);
  mainLayout->addSpacing(5);

  textBack = new KColorButton(colors[0],this);
  label = new QLabel(textBack,i18n("Text Background:"),this);
  label->setFixedSize( label->sizeHint() );
  textBack->setFixedSize( textBack->sizeHint() );
  mainLayout->addWidget(label, 0, AlignLeft);
  mainLayout->addWidget(textBack, 0, AlignLeft);
  mainLayout->addSpacing(5);

  selected = new KColorButton(colors[1],this);
  label = new QLabel(selected,i18n("Selected:"),this);
  label->setFixedSize( label->sizeHint() );
  selected->setFixedSize( selected->sizeHint() );
  mainLayout->addWidget(label, 0, AlignLeft);
  mainLayout->addWidget(selected, 0, AlignLeft);
  mainLayout->addSpacing(5);

  found = new KColorButton(colors[2],this);
  label = new QLabel(found,i18n("Found:"),this);
  label->setFixedSize( label->sizeHint() );
  found->setFixedSize( found->sizeHint() );
  mainLayout->addWidget(label, 0, AlignLeft);
  mainLayout->addWidget(found, 0, AlignLeft);
  mainLayout->addSpacing(5);

  selFound = new KColorButton(colors[3],this);
  label = new QLabel(selFound,i18n("Selected + Found:"),this);
  label->setFixedSize( label->sizeHint() );
  selFound->setFixedSize( selFound->sizeHint() );
  mainLayout->addWidget(label, 0, AlignLeft);
  mainLayout->addWidget(selFound, 0, AlignLeft);
  mainLayout->addSpacing(5);

  QHBoxLayout *hbl = new QHBoxLayout();
  mainLayout->addLayout(hbl);
  hbl->addStretch(1);
  button = new QPushButton(i18n("&OK"),this);
  button->setDefault(true);
  button->setFixedSize( button->sizeHint() );
  hbl->addWidget( button );
  connect(button,SIGNAL(clicked()),this,SLOT(accept()));

  button = new QPushButton(i18n("Cancel"),this);
  button->setFixedSize( button->sizeHint() );
  hbl->addWidget( button );
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));

  mainLayout->activate();
  mainLayout->freeze();
  
//  resize(minimumSize());
}

void ColorDialog::getColors(QColor *colors) {
  colors[4] = back->color();
  colors[0] = textBack->color();
  colors[1] = selected->color();
  colors[2] = found->color();
  colors[3] = selFound->color();
}
#include "kwdialog.moc"
