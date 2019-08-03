
#include <QtWidgets>

#include "egs_editor.h"

EGS_Editor::EGS_Editor(QWidget *parent) : QPlainTextEdit(parent)
{
    this->setFrameShape(QFrame::NoFrame);

    installEventFilter(this);
    viewport()->installEventFilter(this);

    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(autoComplete()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}



int EGS_Editor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}



void EGS_Editor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth()+5, 0, 0, 0);
}



void EGS_Editor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}



void EGS_Editor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}



void EGS_Editor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::lightGray).lighter(120);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}



void EGS_Editor::autoComplete()
{
    QTextCursor cursor = textCursor();
    int clickedPosition = cursor.position();

    // Get the text of the current line
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    QString selectedText = cursor.selectedText();

    if(selectedText.simplified() == "library =") {
        //insertPlainText(selectedText);

        // Init popup
        QListView *popup = new QListView;
        popup->setEditTriggers(QAbstractItemView::NoEditTriggers);
        popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        popup->setSelectionBehavior(QAbstractItemView::SelectRows);
        popup->setSelectionMode(QAbstractItemView::SingleSelection);
        //popup->setModelColumn(d->column);

        popup->setParent(nullptr);
        // This option seems to take control of mouse + key inputs
        // essentially locking up the computer, beware!
        //popup->setWindowFlag(Qt::Popup);
        popup->setWindowFlag(Qt::ToolTip);
        popup->setFocusPolicy(Qt::NoFocus);

        popup->installEventFilter(this);

        QObject::connect(popup, SIGNAL(clicked(QModelIndex)),
                        this, SLOT(insertCompletion(QModelIndex)));
        QObject::connect(this, SIGNAL(cursorPositionChanged()),
                        popup, SLOT(hide()));
//
//         QObject::connect(popup->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//                         this, SLOT(_q_completionSelected(QItemSelection)));

        // Init model
        QStringListModel *model;
        model = new QStringListModel(this);

        // Make data
        QStringList List;
        List << "egs_box" << "egs_cd_geometry" << "egs_cones";
        model->setStringList(List);

        popup->setModel(model);

        // Create a selection popup
        QRect rect; //tmp rect
        int maxVisibleItems = 6;
        const QRect screen = this->frameRect();
        Qt::LayoutDirection dir = this->layoutDirection();
        QPoint pos;
        int rh, w;
        int h = (popup->sizeHintForRow(0) * qMin(maxVisibleItems, popup->model()->rowCount()) + 3) + 3;
        QScrollBar *hsb = popup->horizontalScrollBar();
        if (hsb && hsb->isVisible())
            h += popup->horizontalScrollBar()->sizeHint().height();

        if (rect.isValid()) {
            rh = rect.height();
            w = rect.width();
            pos = this->mapToGlobal(dir == Qt::RightToLeft ? rect.bottomRight() : rect.bottomLeft());
        } else {
            rh = this->height();
            pos = this->mapToGlobal(QPoint(0, this->height() - 2));
            w = this->width();
        }

        // Constrain the box size to the window
        if (w > screen.width())
            w = screen.width();
        if ((pos.x() + w) > (screen.x() + screen.width()))
            pos.setX(screen.x() + screen.width() - w);
        if (pos.x() < screen.x())
            pos.setX(screen.x());

        int top = pos.y() - rh - screen.top() + 2;
        int bottom = screen.bottom() - pos.y();
        h = qMax(h, popup->minimumHeight());
        if (h > bottom) {
            h = qMin(qMax(top, bottom), h);

            if (top > bottom)
                pos.setY(pos.y() - h - rh + 2);
        }

        popup->setGeometry(pos.x(), pos.y(), w, h);

        // Show the popup
        if (!popup->isVisible())
            popup->show();
    }
}

void EGS_Editor::insertCompletion(QModelIndex index) {
    insertPlainText("test");
    //insertPlainText(index);
}



void EGS_Editor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    //painter.fillRect(event->rect(), QColor(Qt::lightGray).lighter(110));


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

bool EGS_Editor::eventFilter(QObject *obj, QEvent *event) {

    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        // track `Ctrl + Click` in the text edit
        if ((obj == this->viewport()) &&
            (mouseEvent->button() == Qt::LeftButton) &&
            (QGuiApplication::keyboardModifiers() == Qt::ControlModifier)) {
            // open the link (if any) at the current position
            //openLinkAtCursorPosition();
            return true;
        }
    }

    return QPlainTextEdit::eventFilter(obj, event);
}

//bool EGS_Editor::openLinkAtCursorPosition() {
//    QTextCursor cursor = this->textCursor();
//    int clickedPosition = cursor.position();

//    // select the text in the clicked block and find out on
//    // which position we clicked
//    cursor.movePosition(QTextCursor::StartOfBlock);
//    int positionFromStart = clickedPosition - cursor.position();
//    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

//    QString selectedText = cursor.selectedText();

//    // find out which url in the selected text was clicked
//    QString urlString = getMarkdownUrlAtPosition(selectedText,
//                                                 positionFromStart);
//    QUrl url = QUrl(urlString);
//    bool isRelativeFileUrl = urlString.startsWith("file://..");

//    qDebug() << __func__ << " - 'emit urlClicked( urlString )': "
//             << urlString;

//    emit urlClicked(urlString);

//    if ((url.isValid() && isValidUrl(urlString)) || isRelativeFileUrl) {
//        // ignore some schemata
//        if (!(_ignoredClickUrlSchemata.contains(url.scheme()) ||
//                isRelativeFileUrl)) {
//            // open the url
//            openUrl(urlString);
//        }

//        return true;
//    }

//    return false;
//}

