/*
###############################################################################
#
#  EGSnrc egs++ egsinp editor
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
#  Author:          Reid Townson, 2020
#
#  Contributors:    
#
###############################################################################
*/

#ifndef EGS_EDITOR_H
#define EGS_EDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QEvent>
#include <QModelIndex>
#include <QtWidgets>

#include "egs_input_struct.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class LineNumberArea;

class EGS_Editor : public QPlainTextEdit {
    Q_OBJECT

public:
    EGS_Editor(QWidget *parent = 0);
    ~EGS_Editor();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void setInputStruct(shared_ptr<EGS_InputStruct> inp);
    void validateEntireInput();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void validateLine(QTextCursor line);
    void autoComplete();
    void insertCompletion(QModelIndex index);
    void updateLineNumberArea(const QRect &, int);

private:
    shared_ptr<EGS_BlockInput> getBlockInput(QString &blockTitle, QTextCursor cursor = QTextCursor());
    QString getBlockTitle(QTextCursor cursor = QTextCursor());
    QString getInputValue(QString inp, QTextBlock currentBlock, bool &foundTag);
    QTextBlock getBlockEnd(QTextBlock currentBlock);
    bool inputHasDependency(shared_ptr<EGS_SingleInput> inp);
    bool inputDependencySatisfied(shared_ptr<EGS_SingleInput> inp, QTextCursor cursor = QTextCursor());
    QTextBlock findSiblingBlock(QString title, QTextBlock currentBlock);
    int countStartingWhitespace(const QString &s);

    QWidget *lineNumberArea;
    shared_ptr<EGS_InputStruct> inputStruct;
    QListView *popup;
    QStringListModel *model;
};


class LineNumberArea : public QWidget {
public:
    LineNumberArea(EGS_Editor *editor) : QWidget(editor) {
        egsEditor = editor;
    }

    QSize sizeHint() const override {
        return QSize(egsEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        egsEditor->lineNumberAreaPaintEvent(event);
    }

private:
    EGS_Editor *egsEditor;
};

#endif // EGS_EDITOR_H
