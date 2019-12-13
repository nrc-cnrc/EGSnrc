#ifndef EGS_EDITOR_H
#define EGS_EDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QEvent>
#include <QModelIndex>

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

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void autoComplete();
    void insertCompletion(QModelIndex index);
    void updateLineNumberArea(const QRect &, int);

private:
    EGS_BlockInput getBlockInput();

    QWidget *lineNumberArea;
    shared_ptr<EGS_InputStruct> inputStruct;
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
