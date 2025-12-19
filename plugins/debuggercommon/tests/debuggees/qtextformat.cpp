#include <QTextFormat>

int main()
{
    QTextFormat invalidFormat;
    QTextFormat obsoleteFormat(4); // the obsolete QTextFormat::TableFormat
    QTextFormat userFormat(QTextFormat::UserFormat);
    QTextFormat userFormat3(QTextFormat::UserFormat + 3);

    QTextCharFormat emptyCharFormat;

    QTextBlockFormat blockFormat;
    blockFormat.setTopMargin(10);

    QTextCharFormat charFormat;
    charFormat.setFontItalic(true);
    charFormat.setToolTip(QStringLiteral("some tooltip"));
    // FontCapitalization shares its value with the FirstFontProperty alias
    charFormat.setFontCapitalization(QFont::AllUppercase);
    // FontSizeAdjustment shares its value with the FontSizeIncrement alias
    charFormat.setProperty(QTextFormat::FontSizeAdjustment, 1);
    charFormat.setProperty(QTextFormat::UserProperty + 1, 42); // a property without a name

    QTextListFormat listFormat;
    listFormat.setIndent(2);

    QTextFrameFormat frameFormat;
    frameFormat.setBorder(3);

    QTextTableFormat tableFormat;
    tableFormat.setCellPadding(4);

    QTextImageFormat imageFormat;
    imageFormat.setName(QStringLiteral("image.png"));

    QTextCharFormat userObjectFormat;
    userObjectFormat.setObjectType(QTextFormat::UserObject + 1);

    return 0; // line 39
}
