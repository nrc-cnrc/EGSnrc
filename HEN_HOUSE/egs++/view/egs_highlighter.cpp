/*
###############################################################################
#
#  EGSnrc egs++ input highlighter
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Reid Townson, 2019
#
#  Contributors:
#
###############################################################################
*/

#include "egs_highlighter.h"

#include <QTextDocument>
#include <QApplication>
#include <QPalette>
#include <QProcess>
#include <QSettings>
#include <QColor>
#include <QDir>

bool EGS_Highlighter::isDarkMode() const {
#ifdef Q_OS_WIN
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    return settings.value("AppsUseLightTheme", 1).toInt() == 0;

#elif defined(Q_OS_MAC)
    QProcess process;
    process.start("defaults", {"read", "-g", "AppleInterfaceStyle"});
    process.waitForFinished(100);
    QString output = process.readAllStandardOutput().trimmed();
    return output.compare("Dark", Qt::CaseInsensitive) == 0;

#elif defined(Q_OS_LINUX)
    // Try GNOME
    {
        QProcess process;
        process.start("gsettings", {"get", "org.gnome.desktop.interface", "color-scheme"});
        process.waitForFinished(100);
        QString output = process.readAllStandardOutput();
        if (output.contains("dark", Qt::CaseInsensitive))
            return true;
    }
    // Try KDE
    {
        QSettings kdeSettings(QDir::homePath() + "/.config/kdeglobals", QSettings::IniFormat);
        QString colorScheme = kdeSettings.value("General/ColorScheme", "").toString();
        if (colorScheme.contains("dark", Qt::CaseInsensitive))
            return true;
    }
    // Fallback heuristic: check app palette brightness
    QColor bg = QApplication::palette().color(QPalette::Base);
    int brightness = qRound(0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue());
    return brightness < 128;

#else
    // Generic fallback
    QColor bg = QApplication::palette().color(QPalette::Base);
    int brightness = qRound(0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue());
    return brightness < 128;
#endif
}

EGS_Highlighter::EGS_Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    bool dark = isDarkMode();

    // ---------- Define color palettes ----------
    QColor keywordColor   = dark ? QColor("#ff8080") : QColor(Qt::darkRed);
    QColor attrColor      = dark ? QColor("#82aaff") : QColor(Qt::darkBlue);
    QColor numberColor    = dark ? QColor("#c3e88d") : QColor(Qt::darkGreen);
    QColor defColor       = dark ? QColor("#c792ea") : QColor(Qt::darkMagenta);
    QColor nameColor      = dark ? QColor("#82aaff") : QColor(Qt::darkBlue);
    QColor quoteColor     = dark ? QColor("#f07178") : QColor(Qt::darkRed);
    QColor squoteColor    = dark ? QColor("#ff5370") : QColor(Qt::red);
    QColor commentColor   = dark ? QColor("#808080") : QColor(Qt::gray);

    // ---------- Define highlighting rules ----------
    HighlightingRule rule;

    keywordFormat.setForeground(keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << ":(start|stop).*\\S:";
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    attributeFormat.setForeground(attrColor);
    rule.pattern = QRegularExpression(".*=");
    rule.format = attributeFormat;
    highlightingRules.append(rule);

    numberFormat.setForeground(numberColor);
    rule.pattern = QRegularExpression("[+-]?(\\d*\\.)?\\d");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    definitionFormat.setForeground(defColor);
    definitionFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(":(start|stop).*(definition|MC transport parameter|run control|scoring options):");
    rule.format = definitionFormat;
    highlightingRules.append(rule);

    nameFormat.setForeground(nameColor);
    nameFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("( )*name( )*=.*");
    rule.format = nameFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(quoteColor);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    squotationFormat.setForeground(squoteColor);
    rule.pattern = QRegularExpression("\'.*\'");
    rule.format = squotationFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(commentColor);
    rule.pattern = QRegularExpression("(#|//|!)[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(commentColor);
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void EGS_Highlighter::highlightBlock(const QString &text) {
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0);

    //For multi-line comments
    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(commentStartExpression);
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
