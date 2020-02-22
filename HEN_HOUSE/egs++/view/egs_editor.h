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
    shared_ptr<EGS_BlockInput> getBlockInput(QString &blockTitle);
    QString getBlockTitle();
    QString getInputValue(QString inp, QTextBlock currentBlock);
    QTextBlock getBlockEnd(QTextBlock currentBlock);
    bool inputHasDependency(shared_ptr<EGS_SingleInput> inp);
    bool inputDependencySatisfied(shared_ptr<EGS_SingleInput> inp);
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
