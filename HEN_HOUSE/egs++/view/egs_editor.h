#ifndef EGS_EDITOR_H
#define EGS_EDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QEvent>
#include <QModelIndex>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class LineNumberArea;

class EGS_Editor : public QPlainTextEdit
{
    Q_OBJECT

public:
    EGS_Editor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

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
    QWidget *lineNumberArea;
};


class LineNumberArea : public QWidget
{
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
