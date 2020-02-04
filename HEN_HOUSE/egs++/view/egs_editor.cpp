#include "egs_editor.h"
#include "egs_functions.h"

EGS_Editor::EGS_Editor(QWidget *parent) : QPlainTextEdit(parent) {
    this->setFrameShape(QFrame::NoFrame);

    // Capture events
    installEventFilter(this);
    viewport()->installEventFilter(this);

    // Set the font
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    this->setFont(fixedFont);

    // Set the tab width to 4 spaces
    const int tabStop = 4; // 4 characters
    QFontMetrics metrics(fixedFont);
    this->setTabStopWidth(tabStop * metrics.width(' '));

    // Initialize an area for displaying line numbers
    lineNumberArea = new LineNumberArea(this);
    updateLineNumberAreaWidth(0);

    // Highlight the line currently selected by the cursor
    highlightCurrentLine();

    // The standard font format has no underline
    normalFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);

    // The format for invalid inputs
    // Adds a little red squiggly line
    invalidFormat.setUnderlineColor(QColor("red"));
    invalidFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

    // Initialize the auto completion popup
    popup = new QListView;
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

    // Init model
    model = new QStringListModel(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), popup, SLOT(hide()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(autoComplete()));
    connect(popup, SIGNAL(clicked(QModelIndex)), this, SLOT(insertCompletion(QModelIndex)));

//
//         QObject::connect(popup->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//                         this, SLOT(_q_completionSelected(QItemSelection)));


}

EGS_Editor::~EGS_Editor() {
    if(lineNumberArea) {
        delete lineNumberArea;
    }
    if(popup) {
        delete popup;
    }
    if(model) {
        delete model;
    }
}

void EGS_Editor::setInputStruct(shared_ptr<EGS_InputStruct> inp) {
    inputStruct = inp;
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

int EGS_Editor::countStartingWhitespace(const QString &s) {
    int i, l = s.size();
    for(i = 0; i < l && s[i] == ' '; ++i);
    return i;
}

void EGS_Editor::autoComplete() {
    // Get the input structure
    QString blockTitle;
    shared_ptr<EGS_BlockInput> inputBlockTemplate = getBlockInput(blockTitle);

    // Get the text of the current line
    QTextCursor cursor = textCursor();
    QString selectedText = cursor.block().text().simplified();

    // If the first character is a "#", ignore this line
    if(selectedText.startsWith("#")) {
        return;
    }

    // Check the validity of the inputs
    // If this line contains an "=" then it should match a single input
    int equalsPos = selectedText.indexOf("=");
    if(equalsPos != -1) {
        QString inputTag = selectedText.left(equalsPos).simplified();
        QString inputVal = selectedText.right(selectedText.size() - equalsPos - 1).simplified();

        // If we found a template for this type of input block,
        // check that the input tag  (LHS) is valid
        if(inputBlockTemplate) {


            QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();
            QTextEdit::ExtraSelection selection;
            selection.cursor = textCursor();
            selection.cursor.joinPreviousEditBlock();

            // Select the whole line
            selection.cursor.movePosition(QTextCursor::StartOfBlock);
            selection.cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

            // Reset the format to have no red underline
            selection.cursor.setCharFormat(normalFormat);

            // Check that the input block template contains this type of input
            //bool inputValid = inputBlockTemplate->contains(inputTag.toStdString());
            shared_ptr<EGS_SingleInput> inputPtr = inputBlockTemplate->getSingleInput(inputTag.toStdString());
            if(!inputPtr) {
                // Select the input tag
                selection.cursor.movePosition(QTextCursor::StartOfBlock);

                // If whitespace was trimmed from the start of the line,
                // we account for it so only the input tag is underlined
                int originalEqualsPos = cursor.block().text().indexOf("=");
                int numWhitespace = countStartingWhitespace(cursor.block().text());
                if(numWhitespace > 0) {
                    selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, numWhitespace);
                }

                selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, originalEqualsPos - numWhitespace);

                // Set the format to have a red underline
                selection.cursor.setCharFormat(invalidFormat);
            } else {
                // If the input is valid, add the description as a tooltip
                QTextCharFormat newFormat;
                newFormat.setToolTip(QString::fromStdString(inputPtr->getDescription()));

                selection.cursor.setCharFormat(newFormat);
            }

            selection.cursor.endEditBlock();
            extraSelections.append(selection);
            setExtraSelections(extraSelections);
        }

        // Return if the input value (RHS) is already filled
        // This way we only offer options for blank inputs
        if(inputVal != "") {
            return;
        }

        // Create a pop-up if there is a list of possible input values
        if (inputTag == "library") {

            // Get the list of possible libraries for this type of input block
            // I.e. if this is a geometry, only offer geometries as options
            vector<string> vals = inputStruct->getLibraryOptions(blockTitle.toStdString());

            // Populate the popup list
            QStringList itemList;
            for(auto &v: vals) {
                itemList << QString(v.c_str());
            }
            if(itemList.size() > 0) {
                model->setStringList(itemList);
            }

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
            int h = (popup->sizeHintForRow(0) * qMin(maxVisibleItems, popup->model()->rowCount()) + 3) + 3;
            QScrollBar *hsb = popup->horizontalScrollBar();
            if (hsb && hsb->isVisible()) {
                h += popup->horizontalScrollBar()->sizeHint().height();
            }

            QPoint pos = this->viewport()->mapToGlobal(this->cursorRect().bottomRight());
            QFontMetrics fm(popup->font());
            int w = 20 + strLength * fm.horizontalAdvance('9');

            popup->setGeometry(pos.x(), pos.y(), w, h);

            // Show the popup
            if (!popup->isVisible()) {
                popup->show();
            }
        } else {

            // Return if we couldn't find a template for this input block
            if(!inputBlockTemplate) {
                return;
            }

            // Check for this input tag in the template
            shared_ptr<EGS_SingleInput> inp = inputBlockTemplate->getSingleInput(inputTag.toStdString());

            // Return if we didn't find this input in the template
            if(!inp) {
                return;
            }

            // Get the possible values
            auto vals = inp->getValues();

            // Return if we don't have a list of values to choose from
            if(vals.size() == 0) {
                return;
            }

            // Populate the popup list
            QStringList itemList;
            for(auto &v: vals) {
                itemList << QString(v.c_str());
            }
            if(itemList.size() > 0) {
                model->setStringList(itemList);
            }

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
            int h = (popup->sizeHintForRow(0) * qMin(maxVisibleItems, popup->model()->rowCount()) + 3) + 3;
            QScrollBar *hsb = popup->horizontalScrollBar();
            if (hsb && hsb->isVisible()) {
                h += popup->horizontalScrollBar()->sizeHint().height();
            }

            QPoint pos = this->viewport()->mapToGlobal(this->cursorRect().bottomRight());
            QFontMetrics fm(popup->font());
            int w = 20 + strLength * fm.horizontalAdvance('9');

            popup->setGeometry(pos.x(), pos.y(), w, h);

            // Show the popup
            if (!popup->isVisible()) {
                popup->show();
            }
        }
    }
}

void EGS_Editor::insertCompletion(QModelIndex index) {
    insertPlainText(model->data(index).toString());
}

shared_ptr<EGS_BlockInput> EGS_Editor::getBlockInput(QString &blockTitle) {
    QString library;
    vector<QString> stopList;
    bool withinOtherBlock = false;
    for(QTextBlock block = textCursor().block(); block.isValid(); block = block.previous()) {
        QString line = block.text().simplified();

        // Get block library for input blocks based on a shared library
        // e.g. geometries and sources
        // Only look for the library tag if we're not in a sub-block
        int pos;
        if(!withinOtherBlock) {
            pos = line.lastIndexOf("library =");
            if(pos >= 0) {
                pos += 9;
                library = line.mid(pos, line.size()-pos).simplified();
            }
        }

        // Get block title
        pos = line.lastIndexOf(":start ");
        if(pos >= 0) {
            pos += 7;
            int endPos = line.indexOf(":",pos);
            if(endPos > 0) {
                blockTitle = line.mid(pos, endPos-pos);
                if(stopList.size() > 0 && blockTitle == stopList.back()) {
                    stopList.pop_back();
                    blockTitle.clear();
                    withinOtherBlock = false;
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
                withinOtherBlock = true;
            }
        }
    }

    // If we got the library tag, we can directly look up this input block structure
    if(library.size() > 0) {
        shared_ptr<EGS_BlockInput> inputBlock = inputStruct->getLibraryBlock(blockTitle.toStdString(), library.toStdString());
        if(inputBlock) {
            return inputBlock;
        }
    }

    return nullptr;
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
    } else if(event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // Insert 4 spaces instead of tabs
        if (keyEvent->key() == Qt::Key_Tab) {
            insertPlainText("    ");
            return true;
        } else if(keyEvent->key() == Qt::Key_Backtab) {
            // Delete 4 spaces from the front of the line
            QTextCursor cursor = textCursor();
            QString line = cursor.block().text();
            if(line.startsWith("    ")) {
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
                cursor.removeSelectedText();
            } else if(line.startsWith("\t")) {
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
                cursor.removeSelectedText();
            }
            return true;
        }
    } else if(event->type() == QEvent::Wheel || event->type() == QEvent::FocusOut) {
        popup->hide();
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

