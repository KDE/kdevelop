void SettingsWidget::changeMessageFontSlot()
{
    KFontDialog::getFont(_messageFont, false, this, true);
    changeMessageFontButton->setText(_messageFont.family());
    changeMessageFontButton->setFont(_messageFont);
}

void SettingsWidget::changeApplicationFontSlot()
{
    KFontDialog::getFont(_applicationFont, false, this, true);
    changeApplicationFontButton->setText(_applicationFont.family());
    changeApplicationFontButton->setFont(_applicationFont);
}


QFont SettingsWidget::applicationFont()
{
    return _applicationFont;
}


QFont SettingsWidget::messageFont()
{
    return _messageFont;
}


void SettingsWidget::setMessageFont( const QFont & fnt )
{
    _messageFont = fnt;
}

void SettingsWidget::setApplicationFont( const QFont & fnt )
{
    _applicationFont = fnt;
}
