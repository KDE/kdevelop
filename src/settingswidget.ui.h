void SettingsWidget::changeMessageFontSlot()
{
    KFontDialog::getFont(messageFont, false, this, true);
    changeMessageFontButton->setText(messageFont.family());
    changeMessageFontButton->setFont(messageFont);
}

void SettingsWidget::changeApplicationFontSlot()
{
    KFontDialog::getFont(applicationFont, false, this, true);
    changeApplicationFontButton->setText(applicationFont.family());
    changeApplicationFontButton->setFont(applicationFont);
}
