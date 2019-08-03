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

#ifndef EGS_HIGHLIGHTER_H
#define EGS_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class QTextDocument;

class EGS_Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit EGS_Highlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression  commentStartExpression,
                        commentEndExpression;

    QTextCharFormat keywordFormat,
                    attributeFormat,
                    numberFormat,
                    definitionFormat,
                    nameFormat,
                    singleLineCommentFormat,
                    quotationFormat,
                    squotationFormat,
                    functionFormat;
    //QTextCharFormat multiLineCommentFormat;

signals:

public slots:

};

#endif // EGS_HIGHLIGHTER_H
