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


SettingsDialog::SettingsDialog(int flags, int wrapAt, int tabWidth, int undoSteps,
  QWidget *parent, const char *name)
  : QDialog(parent,name,true) {

  QLabel *label1;
  QLabel *label2;
  QLabel *label3;
  QPushButton *button1, *button2;
  char buf[8];

  QGroupBox *g1 = new QGroupBox(i18n("Edit Options"),this);
  opt1 = new QCheckBox(i18n("Auto &Indent"),g1);
  opt2 = new QCheckBox(i18n("&Backspace Indent"),g1);
  opt3 = new QCheckBox(i18n("&Word Wrap"),g1);
  opt4 = new QCheckBox(i18n("&Replace Tabs"),g1);
  opt5 = new QCheckBox(i18n("Remove Trailing &Spaces"),g1);
  opt6 = new QCheckBox(i18n("Wrap &Cursor"),g1);
  opt7 = new QCheckBox(i18n("&Auto Brackets"),g1);
  opt15 = new QCheckBox(i18n("&Highlight Brackets"),g1);
  opt8 = new QCheckBox(i18n("&Show tabs"), g1);

//  g1->setMinimumHeight(8+8+8+6*4+9*(opt1->sizeHint().height()));

  opt1->setChecked(flags & cfAutoIndent);
  opt2->setChecked(flags & cfBackspaceIndent);
  opt3->setChecked(flags & cfWordWrap);
  opt4->setChecked(flags & cfReplaceTabs);
  opt5->setChecked(flags & cfRemoveSpaces);
  opt6->setChecked(flags & cfWrapCursor);
  opt7->setChecked(flags & cfAutoBrackets);
  opt8->setChecked(flags & cfShowTabs);

  QGroupBox *g2 = new QGroupBox(i18n("Select Options"),this);
  opt9 = new QCheckBox(i18n("&Persistent Selections"),g2);
  opt10 = new QCheckBox(i18n("&Multiple Selections"),g2);
  opt11 = new QCheckBox(i18n("&Vertical Selections"),g2);
  opt12 = new QCheckBox(i18n("&Delete On Input"),g2);
  opt13 = new QCheckBox(i18n("&Toggle Old"),g2);
  opt14 = new QCheckBox(i18n("A&uto Copy"),g2);

  g2->setMinimumHeight(8+8+8+6*4+6*(opt9->sizeHint().height()));

  opt9->setChecked(flags & cfPersistent);
  opt10->setChecked(flags & cfKeepSelection);
  opt11->setChecked(flags & cfVerticalSelect);
  opt12->setChecked(flags & cfDelOnInput);
  opt13->setChecked(flags & cfXorSelect);
  opt14->setChecked(flags & cfAutoCopy);
  opt15->setChecked(flags & cfHighlightBrackets);

  e1 = new QLineEdit(this);
  sprintf(buf,"%d",wrapAt);
  e1->setText(buf);
  e1->setValidator( new KIntValidator( e1 ) );
  label1 = new QLabel(e1,i18n("Wrap Words At:"),this);
  int max = label1->sizeHint().width();

  e2 = new QLineEdit(this);
  sprintf(buf,"%d",tabWidth);
  e2->setText(buf);
  e2->setValidator( new KIntValidator( e2 ) );
  label2 = new QLabel(e2,i18n("Tab Width:"),this);
  if (label2->sizeHint().width()>max)
    max = label2->sizeHint().width();

  e3 = new QLineEdit(this);
  sprintf(buf,"%d",undoSteps);
  e3->setText(buf);
  e3->setValidator( new KIntValidator( e3 ) );
  label3 = new QLabel(e3,i18n("Undo steps:"),this);
  if (label3->sizeHint().width()>max)
    max = label3->sizeHint().width();

  label1->setFixedSize( max, label1->sizeHint().height() );
  label2->setFixedSize( max, label2->sizeHint().height() );
  label3->setFixedSize( max, label3->sizeHint().height() );

  e1->setFixedSize( max, e1->sizeHint().height() );
  e2->setFixedSize( max, e2->sizeHint().height() );
  e3->setFixedSize( max, e3->sizeHint().height() );
  
  button1 = new QPushButton(i18n("&OK"),this);
  button1->setFixedSize(button1->sizeHint());
  button1->setDefault(true);
  connect(button1,SIGNAL(clicked()),this,SLOT(accept()));

  button2 = new QPushButton(i18n("Cancel"),this);
  button2->setFixedSize(button2->sizeHint());
  connect(button2,SIGNAL(clicked()),this,SLOT(reject()));


  QVBoxLayout *mainLayout = new QVBoxLayout(this, 8, 4);

  QHBoxLayout *vbl6 = new QHBoxLayout();
  mainLayout->addLayout( vbl6 );

  QVBoxLayout *vbl3 = new QVBoxLayout();
  vbl6->addLayout( vbl3 );

  vbl3->addWidget( g1 );

  QVBoxLayout *vbl1 = new QVBoxLayout(g1, 8, 4);
  vbl1->addSpacing(8);
  vbl1->addWidget( opt1,0,AlignLeft );
  vbl1->addWidget( opt2,0,AlignLeft );
  vbl1->addWidget( opt3,0,AlignLeft );
  vbl1->addWidget( opt4,0,AlignLeft );
  vbl1->addWidget( opt5,0,AlignLeft );
  vbl1->addWidget( opt6,0,AlignLeft );
  vbl1->addWidget( opt7,0,AlignLeft );
  vbl1->addWidget( opt15,0,AlignLeft );
  vbl1->addWidget( opt8,0,AlignLeft );

  vbl3->addSpacing( 10 );
  vbl3->addWidget( g2 );

  QVBoxLayout *vbl2 = new QVBoxLayout(g2, 8, 4);
  vbl2->addSpacing(8);
  vbl2->addWidget( opt9,0,AlignLeft );
  vbl2->addWidget( opt10,0,AlignLeft );
  vbl2->addWidget( opt11,0,AlignLeft );
  vbl2->addWidget( opt12,0,AlignLeft );
  vbl2->addWidget( opt13,0,AlignLeft );
  vbl2->addWidget( opt14,0,AlignLeft );

  vbl6->addSpacing( 10 );

  QVBoxLayout *vbl5 = new QVBoxLayout();
  vbl6->addLayout( vbl5 );

  vbl5->addWidget( label1,0,AlignLeft );
  vbl5->addWidget( e1,0,AlignLeft );
  vbl5->addSpacing( 20 );
  vbl5->addWidget( label2,0,AlignLeft );
  vbl5->addWidget( e2,0,AlignLeft );
  vbl5->addSpacing( 20 );
  vbl5->addWidget( label3,0,AlignLeft );
  vbl5->addWidget( e3,0,AlignLeft );

  mainLayout->addSpacing( 10 );
  mainLayout->addStretch( 1 );

  QHBoxLayout *vbl4 = new QHBoxLayout();
  mainLayout->addLayout( vbl4 );
  vbl4->addStretch( 1 );
  vbl4->addWidget(button1);
  vbl4->addWidget(button2);

  mainLayout->activate();
  resize(minimumSize());

}

int SettingsDialog::getFlags() {
  int flags;

  flags = 0;
  if (opt1->isChecked()) flags |= cfAutoIndent;
  if (opt2->isChecked()) flags |= cfBackspaceIndent;
  if (opt3->isChecked()) flags |= cfWordWrap;
  if (opt4->isChecked()) flags |= cfReplaceTabs;
  if (opt5->isChecked()) flags |= cfRemoveSpaces;
  if (opt6->isChecked()) flags |= cfWrapCursor;
  if (opt7->isChecked()) flags |= cfAutoBrackets;
  if (opt8->isChecked()) flags |= cfShowTabs;

  if (opt9->isChecked()) flags |= cfPersistent;
  if (opt10->isChecked()) flags |= cfKeepSelection;
  if (opt11->isChecked()) flags |= cfVerticalSelect;
  if (opt12->isChecked()) flags |= cfDelOnInput;
  if (opt13->isChecked()) flags |= cfXorSelect;
  if (opt14->isChecked()) flags |= cfAutoCopy;
  if (opt15->isChecked()) flags |= cfHighlightBrackets;
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
