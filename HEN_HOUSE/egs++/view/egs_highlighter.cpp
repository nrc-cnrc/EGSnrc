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

EGS_Highlighter::EGS_Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {

    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkRed);
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywordPatterns;
    keywordPatterns << ":(start|stop).*\\S:";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    attributeFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegularExpression(".*=");
    rule.format = attributeFormat;
    highlightingRules.append(rule);

    numberFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("[+-]?(\\d*\\.)?\\d");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    definitionFormat.setForeground(Qt::darkMagenta);
    definitionFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(":(start|stop).*(definition|MC transport parameter|run control|scoring options):");
    rule.format = definitionFormat;
    highlightingRules.append(rule);

    nameFormat.setForeground(Qt::darkBlue);
    nameFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("( )*name( )*=.*");
    rule.format = nameFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::darkRed);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    squotationFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression("\'.*\'");
    rule.format = squotationFormat;
    highlightingRules.append(rule);

    // Comment highlighting must come last
    singleLineCommentFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression("(#|//|!)[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // For multi-line comments
    multiLineCommentFormat.setForeground(Qt::gray);
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
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
