/*
    SPDX-FileCopyrightText: 2020 Jonathan L. Verner <jonathan.verner@matfyz.cz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "simplecommitform.h"

#include <KColorScheme>
#include <KLocalizedString>
#include <KMessageWidget>

#include <QAction>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>

QPixmap textIcon(const QString& str, const QColor& fg = Qt::black, const QColor& bg = Qt::white)
{
    QPixmap ret(40, 40);
    QPainter p(&ret);
    QFont font = p.font();
    font.setPixelSize(0.625 * 40);
    p.setFont(font);
    ret.fill(bg);
    p.setPen(fg);
    p.drawText(ret.rect(), Qt::AlignCenter, str);
    return ret;
}

SimpleCommitForm::SimpleCommitForm(QWidget* parent)
    : QWidget(parent)
    , m_commitBtn(new QPushButton(i18n("Commit")))
    , m_summaryEdit(new QLineEdit)
    , m_messageEdit(new QTextEdit)
    , m_inlineError(new KMessageWidget)
    , m_disabled(false)
{
    QHBoxLayout* _hlayout = new QHBoxLayout();
    _hlayout->setSpacing(0);
    _hlayout->addWidget(m_summaryEdit);
    _hlayout->addWidget(m_commitBtn);

    QVBoxLayout* _vlayout = new QVBoxLayout(this);
    _vlayout->setSpacing(2);
    _vlayout->setMargin(0);
    _vlayout->addWidget(m_inlineError);
    _vlayout->addLayout(_hlayout);
    _vlayout->addWidget(m_messageEdit);

    m_inlineError->setHidden(true);
    m_inlineError->setMessageType(KMessageWidget::Error);
    m_inlineError->setCloseButtonVisible(true);
    m_inlineError->setWordWrap(true);

    auto monospace = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monospace.setStyleHint(QFont::TypeWriter);
    m_messageEdit->setPlaceholderText(i18n("Extended commit description"));
    m_messageEdit->setMinimumHeight(30);
    m_messageEdit->setFont(monospace);
    m_messageEdit->setWordWrapMode(QTextOption::WordWrap);
    m_messageEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
    m_messageEdit->setLineWrapColumnOrWidth(80);

    QAction* summaryLen = new QAction(this);
    m_summaryEdit->setPlaceholderText(i18n("Commit summary"));
    m_summaryEdit->addAction(summaryLen, QLineEdit::TrailingPosition);
    m_summaryEdit->setFont(monospace);

    auto colors = KColorScheme();

    summaryLen->setIcon(textIcon(QString::number(0), colors.foreground(KColorScheme::NormalText).color(),
                                 colors.background(KColorScheme::NormalBackground).color()));
    connect(m_summaryEdit, &QLineEdit::textChanged, this, [=] {
        int sz = m_summaryEdit->text().size();

        // Disable the commit button if the message is empty
        if (sz == 0) {
            m_commitBtn->setDisabled(true);
            if (!m_disabled)
                m_commitBtn->setToolTip(i18n("To commit changes, please write a commit message first"));
        } else if (!m_disabled) {
            m_commitBtn->setToolTip(
                i18n("Commit changes to <b>%1</b> on branch <b>%2</b>", m_projectName, m_branchName));
            m_commitBtn->setDisabled(false);
        }

        // Determine the summary background color based on the length
        QBrush fg, bg;

        if (sz < 65) {
            bg = colors.background(KColorScheme::NormalBackground);
            fg = colors.foreground(KColorScheme::NormalText);
        } else if (sz < 73) {
           bg = colors.background(KColorScheme::NormalBackground);
           fg = colors.foreground(KColorScheme::NormalText);
        } else if (sz < 79) {
            bg = colors.background(KColorScheme::NeutralBackground);
            fg = colors.foreground(KColorScheme::NeutralText);
        } else {
            bg = colors.background(KColorScheme::NegativeBackground);
            fg = colors.foreground(KColorScheme::NegativeText);
        }

        QPalette palette = QGuiApplication::palette();
        palette.setBrush(QPalette::Base, bg);
        palette.setBrush(QPalette::Text, fg);
        m_summaryEdit->setPalette(palette);
        summaryLen->setIcon(textIcon(QString::number(sz), fg.color(), bg.color()));
    });

    connect(m_commitBtn, &QPushButton::clicked, this, &SimpleCommitForm::committed);
    setLayout(_vlayout);
    m_commitBtn->setDisabled(true);
    m_commitBtn->setToolTip(i18n("To commit changes, please write a commit message first"));
}

QString SimpleCommitForm::extendedDescription(int wrapAtColumn) const
{
    if (wrapAtColumn <= 0)
        return m_messageEdit->toPlainText();
    int currentLineLen = 0;
    QString ret;
    for(const auto c: m_messageEdit->toPlainText()) {
        if (c == QLatin1Char('\n')) {
            ret += QLatin1Char('\n');
            currentLineLen = 0;
        } else if (currentLineLen > wrapAtColumn && c.isSpace()) {
            ret += QLatin1Char('\n');
            currentLineLen = 0;
        } else {
            ret += c;
            currentLineLen++;
        }
    }
    return ret;
}

void SimpleCommitForm::setExtendedDescription(const QString& txt)
{
    m_messageEdit->setText(txt);
}
QString SimpleCommitForm::summary() const
{
    return m_summaryEdit->text();
}
void SimpleCommitForm::setSummary(const QString& txt)
{
    m_summaryEdit->setText(txt);
}
void SimpleCommitForm::disableCommitButton()
{
    m_disabled = true;
    m_commitBtn->setDisabled(true);
    m_commitBtn->setToolTip(i18n("Please stage some changes before committing."));
}
void SimpleCommitForm::enableCommitButton()
{
    m_disabled = false;
    if (m_summaryEdit->text().size() > 0) {
        m_commitBtn->setToolTip(i18n("Commit changes to <b>%1</b> on branch <b>%2</b>", m_projectName, m_branchName));
        m_commitBtn->setDisabled(false);
    } else {
        m_commitBtn->setToolTip(i18n("To commit changes, please write a commit message first"));
    }
}

void SimpleCommitForm::disable()
{
    m_commitBtn->setDisabled(true);
    m_summaryEdit->setDisabled(true);
    m_messageEdit->setDisabled(true);
}

void SimpleCommitForm::enable()
{
    if (! m_disabled)
        enableCommitButton();
    m_summaryEdit->setDisabled(false);
    m_messageEdit->setDisabled(false);
}



void SimpleCommitForm::clear()
{
    m_summaryEdit->clear();
    m_messageEdit->clear();
    clearError();
}


void SimpleCommitForm::setBranchName(const QString& branchName)
{
    m_branchName = branchName;
}

void SimpleCommitForm::setProjectName(const QString& projName)
{
    m_projectName = projName;
}

void SimpleCommitForm::showError(const QString& error)
{
    m_inlineError->setText(error);
    m_inlineError->animatedShow();
}

void SimpleCommitForm::clearError()
{
    if (!m_inlineError->isHidden() && !m_inlineError->isHideAnimationRunning()) {
        m_inlineError->animatedHide();
    }
}
