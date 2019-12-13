
#include <QtWidgets>

#include "egs_editor.h"
#include "egs_functions.h"

EGS_Editor::EGS_Editor(QWidget *parent) : QPlainTextEdit(parent) {
    this->setFrameShape(QFrame::NoFrame);

    installEventFilter(this);
    viewport()->installEventFilter(this);

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    this->setFont(fixedFont);

    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(autoComplete()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

EGS_Editor::~EGS_Editor() {
    if(lineNumberArea) {
        delete lineNumberArea;
    }
}

void EGS_Editor::setInputStruct(shared_ptr<EGS_InputStruct> inp) {
    cout << "test EGS_Editor::setInputStruct" << endl;
    inputStruct = *inp;
    shared_ptr<EGS_BlockInput> libraryBlock = inputStruct.getLibraryBlock("","");
}

int EGS_Editor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}



void EGS_Editor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth()+5, 0, 0, 0);
}



void EGS_Editor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy) {
        lineNumberArea->scroll(0, dy);
    }
    else {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}



void EGS_Editor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}



void EGS_Editor::highlightCurrentLine() {
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



void EGS_Editor::autoComplete() {
    // Get the input structure
    EGS_BlockInput inp = getBlockInput();

    // Get the text of the current line
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    QString selectedText = cursor.selectedText();

    if (selectedText.simplified() == "library = ") {

        // Init popup
        QListView *popup = new QListView;
        popup->setEditTriggers(QAbstractItemView::NoEditTriggers);
        popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        popup->setSelectionBehavior(QAbstractItemView::SelectRows);
        popup->setSelectionMode(QAbstractItemView::SingleSelection);
        popup->setParent(nullptr);
        popup->setFocusPolicy(Qt::NoFocus);
        popup->installEventFilter(this);

        // The Qt::Popup option seems to take control of mouse + key inputs
        // essentially locking up the computer, beware!
        //popup->setWindowFlag(Qt::Popup);
        popup->setWindowFlag(Qt::ToolTip);

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
        QStringList itemList;
        itemList << "egs_box" << "egs_cd_geometry" << "egs_cones" << "eii_iii";
        model->setStringList(itemList);

        popup->setModel(model);
        popup->setFont(this->font());

        // Get max string length
        int strLength = 0;
        for (auto &item: itemList) {
            if (item.size() > strLength) {
                strLength = item.size();
            }
        }

        // Create a selection popup
        int maxVisibleItems = 6;
        const QRect screen = this->frameRect();
        QPoint pos;
        int rh, w;
        int h = (popup->sizeHintForRow(0) * qMin(maxVisibleItems, popup->model()->rowCount()) + 3) + 3;
        QScrollBar *hsb = popup->horizontalScrollBar();
        if (hsb && hsb->isVisible()) {
            h += popup->horizontalScrollBar()->sizeHint().height();
        }

        rh = this->height();
        pos = this->viewport()->mapToGlobal(this->cursorRect().bottomRight());
        QFontMetrics fm(popup->font());
        w = 20 + strLength * fm.horizontalAdvance('9');

        popup->setGeometry(pos.x(), pos.y(), w, h);

        // Show the popup
        if (!popup->isVisible()) {
            popup->show();
        }
    }
}

void EGS_Editor::insertCompletion(QModelIndex index) {
    insertPlainText("test");
    //insertPlainText(index);
}

// TODO: on clicking a new position in doc
// - get nearest :start, load inputstruct (for geom/src, get library first)
EGS_BlockInput EGS_Editor::getBlockInput() {
    cout << "test getBlockInput " << endl;
    shared_ptr<EGS_BlockInput> libraryBlock = inputStruct.getLibraryBlock("","");

    QString library, blockTitle;
    vector<QString> stopList;
    for(QTextBlock block = textCursor().block(); block.isValid(); block = block.previous()) {
        QString line = block.text().simplified();

        // Get block library for input blocks based on a shared library
        // e.g. geometries and sources
        int pos = line.lastIndexOf("library =");
        if(pos >= 0) {
            pos += 9;
            library = line.mid(pos, line.size()-pos).simplified();
            cout << "test1 " << library.toLatin1().data() << endl;
        }

        // Get block title
        pos = line.lastIndexOf(":start ");
        if(pos >= 0) {
            pos += 7;
            int endPos = line.indexOf(":",pos);
            if(endPos > 0) {
                blockTitle = line.mid(pos, endPos-pos);
                //cout << "test2 " << blockTitle.toLatin1().data() << endl;
                if(stopList.size() > 0 && blockTitle == stopList.back()) {
                    stopList.pop_back();
                    blockTitle.clear();
                } else {
                    break;
                }
            }
        }

        // Save a vector of blocks that have already been closed
        // This means both a matching :start and :stop are above the cursor
        // so we're not inside the block
        pos = line.lastIndexOf(":stop ");
        if(pos >= 0) {
            pos += 6;
            int endPos = line.indexOf(":",pos);
            if(endPos > 0) {
                QString stopTitle = line.mid(pos, endPos-pos);
                stopList.push_back(stopTitle);
            }
        }
    }

    // If we got the library tag, we can directly look up this input block structure
    shared_ptr<EGS_BlockInput> inputBlock;
    cout << "test4a " << blockTitle.toLatin1().data() << endl;

//     if(library.size() > 0) {
//         cout << "test3a " << endl;
//         inputBlock = inputStruct->getLibraryBlock(blockTitle.toStdString(), library.toStdString());
//         cout << "test3b " << endl;
//         if(inputBlock) {
//             cout << "test3 " << inputBlock->getTitle().c_str() << endl;
//         }
//     }
    cout << "test4b " << endl;


    return EGS_BlockInput();
}

void EGS_Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
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

