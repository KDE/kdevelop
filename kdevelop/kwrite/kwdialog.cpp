//Search dialog

#include <stdlib.h>
#include <stdio.h>

#include <qlabel.h>
#include <qbttngrp.h>
#include <qpushbt.h>

#include <kapp.h>

#include "kwdialog.h"
#include "kwdoc.h"

SearchDialog::SearchDialog(const char *searchFor, const char *replaceWith,
  int flags, QWidget *parent, const char *name)
  : QDialog(parent,name,true) {

  QButtonGroup *group;
  QLabel *label;
  QPushButton *button;
  QRect r, br;
  int y;

//  setFocusPolicy(QWidget::StrongFocus);

  search = new QLineEdit(this);
  search->setText(searchFor);
  search->selectAll();
  label = new QLabel(search,i18n("&Text To Find:"),this);

  r.setRect(10,0,300,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  search->setGeometry(r);

  if (flags & sfReplace) {
    replace = new QLineEdit(this);
    replace->setText(replaceWith);
    label = new QLabel(replace,i18n("&Replace With:"),this);

    r.moveBy(0,25);
    label->setGeometry(r);
    r.moveBy(0,25);
    replace->setGeometry(r);
  } else replace = 0;

  y = r.bottom() + 10;

  group = new QButtonGroup(i18n("Options"),this);
  opt1 = new QCheckBox(i18n("&Case Sensitive"),group);
  opt2 = new QCheckBox(i18n("&Whole Words Only"),group);
  opt3 = new QCheckBox(i18n("&From Cursor"),group);
  opt4 = new QCheckBox(i18n("Find &Backwards"),group);
  opt5 = new QCheckBox(i18n("&Selected Text"),group);

  opt1->setChecked(flags & sfCaseSensitive);
  opt2->setChecked(flags & sfWholeWords);
  opt3->setChecked(flags & sfFromCursor);
  opt4->setChecked(flags & sfBackward);
  opt5->setChecked(flags & sfSelected);

  r.setTop(y);
  r.setHeight(95);
  group->setGeometry(r);

  br.setRect(10,15,r.width()/2 - 20,20);
  opt1->setGeometry(br);
  br.moveBy(0,25);
  opt2->setGeometry(br);
  br.moveBy(0,25);
  opt3->setGeometry(br);

  br.setRect(r.width()/2 + 10,15,r.width()/2 - 20,20);
  opt4->setGeometry(br);
  br.moveBy(0,25);
  opt5->setGeometry(br);

  if (replace) {
    opt6 = new QCheckBox(i18n("&Prompt On Replace"),group);
    opt6->setChecked(flags & sfPrompt);
    br.moveBy(0,25);
    opt6->setGeometry(br);
  }

  y = r.bottom() + 15;

  button = new QPushButton(i18n("&OK"),this);
  button->setDefault(true);
  br.setRect(r.x(),y,60,25);
  button->setGeometry(br);
  connect(button,SIGNAL(clicked()),this,SLOT(okSlot()));

  button = new QPushButton(i18n("Cancel"),this);
  br.moveBy(r.width() - 60,0);
  button->setGeometry(br);
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));//SIGNAL(doneSearch()));

  setFixedSize(r.x()*2 + r.width(),br.bottom() + 5);

  search->setFocus();
}

const char *SearchDialog::getSearchFor() {
  return search->text();
}

const char *SearchDialog::getReplaceWith() {
  return replace->text();
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

  text = search->text();
  if (text && *text) accept();//emit search();
}

ReplacePrompt::ReplacePrompt(QWidget *parent, const char *name)
  : QDialog(0L,name,false) {

  QLabel *label;
  QPushButton *button;
  QRect r;

  label = new QLabel(i18n("Replace this occurence?"),this);

  r.setRect(10,0,200,25);
  label->setGeometry(r);

  button = new QPushButton(i18n("&Yes"),this);
  r.setRect(10,35,50,25);
  button->setGeometry(r);
  button->setDefault(true);
  button->setAutoDefault(true);
  connect(button,SIGNAL(clicked()),this,SLOT(accept()));

  button = new QPushButton(i18n("&No"),this);
  r.moveBy(60,0);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(no()));

  button = new QPushButton(i18n("&All"),this);
  r.moveBy(60,0);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(all()));

  button = new QPushButton(i18n("&Close"),this);
  r.moveBy(60,0);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));

  setFixedSize(r.right() + 10,r.bottom() + 10);

  if (parent) {
    QWidget *w;
    QPoint p(0,0);

    w = parent;//->topLevelWidget();
    p = w->mapToGlobal(p);
    move(p.x() + (w->width() - width())/2,
         p.y() + w->height() - height() - 48);
  }

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
  QRect r, br;
  QLabel *label;
  QPushButton *button;

  e1 = new QLineEdit(this);
  sprintf(buf,"%d",line);
  e1->setText(buf);
  e1->selectAll();
  label = new QLabel(e1,i18n("&Goto Line:"),this);

  r.setRect(10,0,150,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  e1->setGeometry(r);

  button = new QPushButton(i18n("&OK"),this);
  button->setDefault(true);
  br.setRect(r.x(),r.bottom() + 10,60,25);
  button->setGeometry(br);
  connect(button,SIGNAL(clicked()),this,SLOT(accept()));

  button = new QPushButton(i18n("Cancel"),this);
  br.moveBy(r.right() - br.right(),0);
  button->setGeometry(br);
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));

  setFixedSize(r.x()*2 + r.width(),br.bottom() + 5);

  e1->setFocus();
}

int GotoLineDialog::getLine() {
  return atoi(e1->text());
}


SettingsDialog::SettingsDialog(int flags, int wrapAt, int tabWidth, int undoSteps,
  QWidget *parent, const char *name)
  : QDialog(parent,name,true) {

  QButtonGroup *group;
  QLabel *label;
  QPushButton *button;
  QRect r, br;
  char buf[8];

  group = new QButtonGroup(i18n("Edit Options"),this);
  opt1 = new QCheckBox(i18n("Auto &Indent"),group);
  opt2 = new QCheckBox(i18n("&Backspace Indent"),group);
  opt3 = new QCheckBox(i18n("&Word Wrap"),group);
  opt4 = new QCheckBox(i18n("Replace &Tabs"),group);
  opt5 = new QCheckBox(i18n("Remove Trailing &Spaces"),group);
  opt6 = new QCheckBox(i18n("Wrap &Cursor"),group);
  opt7 = new QCheckBox(i18n("&Auto Brackets"),group);

  opt1->setChecked(flags & cfAutoIndent);
  opt2->setChecked(flags & cfBackspaceIndent);
  opt3->setChecked(flags & cfWordWrap);
  opt4->setChecked(flags & cfReplaceTabs);
  opt5->setChecked(flags & cfRemoveSpaces);
  opt6->setChecked(flags & cfWrapCursor);
  opt7->setChecked(flags & cfAutoBrackets);

  br.setRect(10,15,160,20);
  opt1->setGeometry(br);
  br.moveBy(0,25);
  opt2->setGeometry(br);
  br.moveBy(0,25);
  opt3->setGeometry(br);
  br.moveBy(0,25);
  opt4->setGeometry(br);
  br.moveBy(0,25);
  opt5->setGeometry(br);
  br.moveBy(0,25);
  opt6->setGeometry(br);
  br.moveBy(0,25);
  opt7->setGeometry(br);

  r.setRect(10,10,br.width() + 20, br.bottom() + 10);
  group->setGeometry(r);

  group = new QButtonGroup(i18n("Select Options"),this);
  opt8 = new QCheckBox(i18n("&Persistent Selections"),group);
  opt9 = new QCheckBox(i18n("&Multiple Selections"),group);
  opt10 = new QCheckBox(i18n("&Vertical Selections"),group);
  opt11 = new QCheckBox(i18n("&Delete On Input"),group);
  opt12 = new QCheckBox(i18n("&Toggle Old"),group);

  opt8->setChecked(flags & cfPersistent);
  opt9->setChecked(flags & cfKeepSelection);
  opt10->setChecked(flags & cfVerticalSelect);
  opt11->setChecked(flags & cfDelOnInput);
  opt12->setChecked(flags & cfXorSelect);

  br.moveBy(0,15 - br.y());
  opt8->setGeometry(br);
  br.moveBy(0,25);
  opt9->setGeometry(br);
  br.moveBy(0,25);
  opt10->setGeometry(br);
  br.moveBy(0,25);
  opt11->setGeometry(br);
  br.moveBy(0,25);
  opt12->setGeometry(br);

  r.setRect(r.x(),r.bottom() + 10,br.width() + 20, br.bottom() + 10);
  group->setGeometry(r);

  br.setRect(r.x(),r.bottom() + 10,60,25); //for first button

  e1 = new QLineEdit(this);
  sprintf(buf,"%d",wrapAt);
  e1->setText(buf);
  label = new QLabel(e1,i18n("Wrap Words At:"),this);

  r.setRect(r.right() + 20,10,100,25);
  label->setGeometry(r);
  r.moveBy(0,25);
  e1->setGeometry(r);

  e2 = new QLineEdit(this);
  sprintf(buf,"%d",tabWidth);
  e2->setText(buf);
  label = new QLabel(e2,i18n("Tab Width:"),this);

  r.moveBy(0,30);
  label->setGeometry(r);
  r.moveBy(0,25);
  e2->setGeometry(r);

  e3 = new QLineEdit(this);
  sprintf(buf,"%d",undoSteps);
  e3->setText(buf);
  label = new QLabel(e3,i18n("Undo steps:"),this);

  r.moveBy(0,30);
  label->setGeometry(r);
  r.moveBy(0,25);
  e3->setGeometry(r);


  button = new QPushButton(i18n("&OK"),this);
  button->setDefault(true);
  button->setGeometry(br);
  connect(button,SIGNAL(clicked()),this,SLOT(accept()));

  button = new QPushButton(i18n("Cancel"),this);
  br.moveBy(r.right() - br.right(),0);
  button->setGeometry(br);
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));

  setFixedSize(br.right() + 10,br.bottom() + 5);
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

  if (opt8->isChecked()) flags |= cfPersistent;
  if (opt9->isChecked()) flags |= cfKeepSelection;
  if (opt10->isChecked()) flags |= cfVerticalSelect;
  if (opt11->isChecked()) flags |= cfDelOnInput;
  if (opt12->isChecked()) flags |= cfXorSelect;
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
  QLabel *label;
  QPushButton *button;
  QRect r, r2;

  back = new KColorButton(colors[4],this);
  label = new QLabel(back,i18n("Background:"),this);
  r.setRect(10,10,140,25);
  label->setGeometry(r);
  r2.setRect(10,r.y() + 25,80,25);
  back->setGeometry(r2);

  textBack = new KColorButton(colors[0],this);
  label = new QLabel(textBack,i18n("Text Background:"),this);
  r.moveBy(0,50);
  label->setGeometry(r);
  r2.moveBy(0,50);
  textBack->setGeometry(r2);

  selected = new KColorButton(colors[1],this);
  label = new QLabel(selected,i18n("Selected:"),this);
  r.moveBy(0,50);
  label->setGeometry(r);
  r2.moveBy(0,50);
  selected->setGeometry(r2);

  found = new KColorButton(colors[2],this);
  label = new QLabel(found,i18n("Found:"),this);
  r.moveBy(0,50);
  label->setGeometry(r);
  r2.moveBy(0,50);
  found->setGeometry(r2);

  selFound = new KColorButton(colors[3],this);
  label = new QLabel(selFound,i18n("Selected + Found:"),this);
  r.moveBy(0,50);
  label->setGeometry(r);
  r2.moveBy(0,50);
  selFound->setGeometry(r2);

  r.setRect(r.x(),r2.bottom() + 10,60,25);
  button = new QPushButton(i18n("&OK"),this);
  button->setDefault(true);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(accept()));

  button = new QPushButton(i18n("Cancel"),this);
  r.moveBy(100,0);
  button->setGeometry(r);
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));
}

void ColorDialog::getColors(QColor *colors) {
  colors[4] = back->color();
  colors[0] = textBack->color();
  colors[1] = selected->color();
  colors[2] = found->color();
  colors[3] = selFound->color();
}
