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
#  Contributors:    Hannah Gallop
#
###############################################################################
*/

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

    // Initialize the auto completion popup
    popup = new QListView;
    popup->setEditTriggers(QAbstractItemView::NoEditTriggers);
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->setSelectionBehavior(QAbstractItemView::SelectRows);
    popup->setSelectionMode(QAbstractItemView::SingleSelection);
    popup->setParent(nullptr);
    popup->setFocusPolicy(Qt::StrongFocus);
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
    connect(popup, SIGNAL(activated(QModelIndex)), this, SLOT(insertCompletion(QModelIndex)));
//
//         QObject::connect(popup->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//                         this, SLOT(_q_completionSelected(QItemSelection)));
    //connect(this, SIGNAL(keyboardGrabber()), this, SIGNAL(QAbstractItemView::MoveDown));

    popupGrabbing = false;
}

EGS_Editor::~EGS_Editor() {
    if (lineNumberArea) {
        delete lineNumberArea;
    }
    if (popup) {
        delete popup;
    }
    if (model) {
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
    for (i = 0; i < l && s[i] == ' ' || s[i] == '\t'; ++i);
    return i;
}

void EGS_Editor::validateEntireInput() {
    // This actually validates from the current cursor position,
    // but we only call this upon loading the file so the cursor is already at the top
    QTextCursor cursor = textCursor();
    for (QTextBlock it = cursor.document()->begin(); it != cursor.document()->end(); it = it.next()) {
        cursor.movePosition(QTextCursor::NextBlock);
        validateLine(cursor);
    }
}

void EGS_Editor::validateLine(QTextCursor cursor) {
    QString selectedText = cursor.block().text().simplified();

    QString blockTitle;
    shared_ptr<EGS_BlockInput> inputBlockTemplate = getBlockInput(blockTitle, cursor);

    // If we aren't inside an input block, ignore this line
    if (blockTitle.size() < 1) {
        return;
    }

    if (selectedText.startsWith("#")) {
        return;
    }

    // Check the validity of the inputs
    // If this line contains an "=" then it should match a single input
    int equalsPos = selectedText.indexOf("=");
    if (equalsPos != -1) {
        cursor.beginEditBlock();

        QString inputTag = selectedText.left(equalsPos).simplified();
        QString inputVal = selectedText.right(selectedText.size() - equalsPos - 1).simplified();
#ifdef EDITOR_DEBUG
        egsInformation("EGS_Editor::validateLine: Found equals sign: %s\n",inputTag.toLatin1().data());
#endif

        // If we found a template for this type of input block,
        // check that the input tag  (LHS) is valid
        if (inputBlockTemplate) {

            QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();
            QTextEdit::ExtraSelection selection;
            selection.cursor = cursor;
            selection.cursor.joinPreviousEditBlock();

            // Select the whole line
            selection.cursor.movePosition(QTextCursor::StartOfBlock);
            selection.cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

            // Reset the format to have no red underline
            QTextCharFormat format;
            format.setUnderlineStyle(QTextCharFormat::NoUnderline);
            format.setToolTip("");
            selection.cursor.setCharFormat(format);

            // Check that the input block template contains this type of input
            // If the input isn't defined, it will return nullptr
            shared_ptr<EGS_SingleInput> inputPtr = inputBlockTemplate->getSingleInput(inputTag.toStdString(), blockTitle.toStdString());
            if (!inputPtr) {
                // Red underline the input tag
                // Select the input tag

                selection.cursor.movePosition(QTextCursor::StartOfBlock);

                // If whitespace was trimmed from the start of the line,
                // we account for it so only the input tag is underlined
                int originalEqualsPos = cursor.block().text().indexOf("=");
                int numWhitespace = countStartingWhitespace(cursor.block().text());
                if (numWhitespace > 0) {
                    selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, numWhitespace);
                }

                selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, originalEqualsPos - numWhitespace);

                // Set the format to have a red underline
                format.setUnderlineColor(QColor("red"));
                format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            }
            else {

                // Get the description for this input
                string desc = inputPtr->getDescription();

                bool isRequired = inputPtr->getRequired();
                if (isRequired) {
                    desc += "\nRequired.";
                }

                // Check if this input has any dependencies
                // and then confirm that the dependencies are satisfied
                if (inputHasDependency(inputPtr)) {
                    // Add the list of dependencies to the description tooltip
                    desc += "\nDependencies: ";
                    auto vals = inputPtr->getDependencyVal();
                    int i = 0;
                    for (auto &inp: inputPtr->getDependencyInp()) {
                        if (vals[i].size() > 0) {
                            desc += "'" + inp->getTag() + "=" + vals[i] + "' ";
                        }
                        else {
                            desc += "'" + inp->getTag() + "' ";
                        }
                        ++i;
                    }
                    auto depBlock = inputPtr->getDependencyBlock();
                    if (depBlock) {
                        desc += "':start " + depBlock->getTitle() + ":'";
                    }

                    if (inputDependencySatisfied(inputPtr, cursor) == false) {
                        // Red underline the input tag
                        // Select the input tag
                        selection.cursor.movePosition(QTextCursor::StartOfBlock);

                        // If whitespace was trimmed from the start of the line,
                        // we account for it so only the input tag is underlined
                        int originalEqualsPos = cursor.block().text().indexOf("=");
                        int numWhitespace = countStartingWhitespace(cursor.block().text());
                        if (numWhitespace > 0) {
                            selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, numWhitespace);
                        }

                        selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, originalEqualsPos - numWhitespace);

                        // Set the format to have a red underline
                        format.setUnderlineColor(QColor("red"));
                        format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
                    }
                }

                // If the input is valid, add the description as a tooltip
                format.setToolTip(QString::fromStdString(desc));
            }

            selection.cursor.setCharFormat(format);

            selection.cursor.endEditBlock();
            extraSelections.append(selection);
            setExtraSelections(extraSelections);
        }
        cursor.endEditBlock();
    }
}

void EGS_Editor::autoComplete() {
    // Get the text of the current line
    QTextCursor cursor = textCursor();
    QString selectedText = cursor.block().text().simplified();

    // Clear the popup string list
    model->setStringList(QStringList{});
    popup->setModel(model);

    // If the first character is a "#", ignore this line
    if (selectedText.startsWith("#")) {
        return;
    }

    // Get the input structure
    QString blockTitle;
    shared_ptr<EGS_BlockInput> inputBlockTemplate = getBlockInput(blockTitle);

    // If we aren't inside an input block, ignore this line
    if (blockTitle.size() < 1) {
        return;
    }

    // Check the validity of the inputs
    // If this line contains an "=" then it should match a single input
    int equalsPos = selectedText.indexOf("=");
    if (equalsPos != -1) {
        QString inputTag = selectedText.left(equalsPos).simplified();
        QString inputVal = selectedText.right(selectedText.size() - equalsPos - 1).simplified();
#ifdef EDITOR_DEBUG
        egsInformation("EGS_Editor::autoComplete: found equals sign: %s\n",inputTag.toLatin1().data());
#endif

        // Check that the line is valid
        validateLine(cursor);

        // Return if the input value (RHS) is already filled
        // This way we only offer options for blank inputs
        if (inputVal != "") {
            return;
        }

        // Create a pop-up if there is a list of possible input values
        if (inputTag == "library") {

            // Get the list of possible libraries for this type of input block
            // I.e. if this is a geometry, only offer geometries as options
            vector<string> vals = inputStruct->getLibraryOptions(blockTitle.toStdString());

            // Populate the popup list
            QStringList itemList;
            for (auto &v: vals) {
                itemList << QString(v.c_str());
            }
            if (itemList.size() > 0) {
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
        else {

            // Return if we couldn't find a template for this input block
            if (!inputBlockTemplate) {
                return;
            }

            // Check for this input tag in the template
            shared_ptr<EGS_SingleInput> inp = inputBlockTemplate->getSingleInput(inputTag.toStdString(), blockTitle.toStdString());

            // Return if we didn't find this input in the template
            if (!inp) {
                return;
            }

            // Get the possible values
            auto vals = inp->getValues();

            // Return if we don't have a list of values to choose from
            if (vals.size() == 0) {
                return;
            }

            // Populate the popup list
            QStringList itemList;
            for (auto &v: vals) {
                itemList << QString(v.c_str());
            }
            if (itemList.size() > 0) {
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

        // If this is just an empty line, we can offer suggestions of valid inputs
    }
    else if (inputBlockTemplate && selectedText == "") {

        vector<shared_ptr<EGS_SingleInput>> singleInputs = inputBlockTemplate->getSingleInputs(blockTitle.toStdString());

        vector<shared_ptr<EGS_BlockInput>> blockInputs = inputBlockTemplate->getBlockInputs(blockTitle.toStdString());

        //vector<shared_ptr<EGS_BlockInput>> generalInputs = inputBlockTemplate->getGeneralBlock(blockTitle.toStdString());

        // Populate the popup list
        QStringList itemList;

        // Add all the single inputs for the top level block
        for (auto &inp: singleInputs) {
            //if(!egsEquivStr(inp->getTag(), "library")) {
            // Skip any inputs that have a dependency which is not satisfied
            if (inputHasDependency(inp) && inputDependencySatisfied(inp, cursor) == false) {
                continue;
            }

            itemList << QString((inp->getTag() + " = ").c_str());
            //}
        }

        // Store the block titles in a set to remove duplicates
        QSet<QString> blockTitles;
        for (auto &block: blockInputs) {
            blockTitles << QString((":start " + block->getTitle() + ":").c_str());
        }
        for (auto &title: blockTitles) {
            itemList << title;
        }
        if (itemList.size() > 0) {
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

        // If this is the start of an input block, check that it belongs here
    }
    else if (selectedText.contains(":start ")) {

        // If we're inside another input block that we have a template for,
        // Check to this that this is a valid input block to exist here
        if (inputBlockTemplate) {

            // Get the block title
            QString blockTit;
            int pos = selectedText.lastIndexOf(":start ");
            pos += 7;
            int endPos = selectedText.indexOf(":",pos);
            if (endPos > 0) {
                blockTit = selectedText.mid(pos, endPos-pos);
            }

            QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();
            QTextEdit::ExtraSelection selection;
            selection.cursor = cursor;
            selection.cursor.joinPreviousEditBlock();

            // Select the whole line
            selection.cursor.movePosition(QTextCursor::StartOfBlock);
            selection.cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

            // Reset the format to have no red underline
            QTextCharFormat format;
            format.setUnderlineStyle(QTextCharFormat::NoUnderline);

            auto inputPtr = inputBlockTemplate->getBlockInput(blockTit.toStdString());
            if (!inputPtr) {
                // Red underline the input tag
                // Select the input tag
                selection.cursor.movePosition(QTextCursor::StartOfBlock);

                // If whitespace was trimmed from the start of the line,
                // we account for it so only the input tag is underlined
                int originalEqualsPos = cursor.block().text().lastIndexOf(":");
                int numWhitespace = countStartingWhitespace(cursor.block().text());
                if (numWhitespace > 0) {
                    selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, numWhitespace);
                }

                selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, originalEqualsPos - numWhitespace);

                // Set the format to have a red underline
                format.setUnderlineColor(QColor("red"));
                format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            }

            selection.cursor.setCharFormat(format);

            selection.cursor.endEditBlock();
            extraSelections.append(selection);
            setExtraSelections(extraSelections);
        }

        // For geometry, source and ausgab object blocks that don't contain a library line,
        // add 'library =' as an option in the popup
    }
    else if (selectedText.size() == 0 && (egsEquivStr(blockTitle.toStdString(), "geometry") || egsEquivStr(blockTitle.toStdString(), "source")
    || egsEquivStr(blockTitle.toStdString(), "ausgab object"))) {

        // Populate the popup list
        QStringList itemList;
        itemList << "library = ";
        if (itemList.size() > 0) {
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
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    this->moveCursor(QTextCursor::EndOfBlock);
    insertPlainText(model->data(index).toString());
    cursor.endEditBlock();
    popup->QWidget::releaseKeyboard();
}

shared_ptr<EGS_BlockInput> EGS_Editor::getBlockInput(QString &blockTitle, QTextCursor cursor) {
    if (cursor == QTextCursor()) {
        cursor = textCursor();
    }

    blockTitle = getBlockTitle(cursor);
    if (blockTitle.size() < 1) {
        return nullptr;
    }

    bool foundTag;
    QString library = getInputValue("library", cursor.block(), foundTag);

    // If we couldn't find a library tag in the current block,
    // try searching the containing block (if there is one)
    if (library.size() < 1) {
#ifdef EDITOR_DEBUG
        egsInformation("EGS_Editor::getBlockInput: Searching containing block for library: %s\n", blockTitle.toLatin1().data());
#endif

        // If we're currently on a :start line, start searching on the next line
        // so that we're actually starting within the block
        QTextBlock blockEnd;
        blockEnd = cursor.block();
        int loopGuard = 10000;
        int i = 0;
        while (blockEnd.text().contains(":start ")) {
            blockEnd = getBlockEnd(blockEnd.next());
            if (++i > loopGuard) {
                egsInformation("Warning: Encountered infinite loop while processing the input file. Contact the developers to report this bug.\n");
                break;
            }
            if (blockEnd.isValid()) {
                blockEnd = blockEnd.next();
            }
        }
        blockEnd = getBlockEnd(blockEnd);
        if (blockEnd.isValid()) {
            // Go to the line after the end of the current input block
            blockEnd = blockEnd.next();

            // Check for the library tag here
            library = getInputValue("library", blockEnd, foundTag);
        }

        // If we still didn't find the library, search one block higher
        if (library.size() < 1) {
#ifdef EDITOR_DEBUG
            egsInformation("EGS_Editor::getBlockInput: Checking up a level...\n");
#endif
            // If we're currently on a :start line, start searching on the next line
            // so that we're actually starting within the block
            int loopGuard = 10000;
            int i = 0;
            while (blockEnd.text().contains(":start ")) {
                blockEnd = getBlockEnd(blockEnd.next());
                if (++i > loopGuard) {
                    egsInformation("Warning: Encountered infinite loop while processing the input file. Contact the developers to report this bug.\n");
                    break;
                }
                if (blockEnd.isValid()) {
                    blockEnd = blockEnd.next();
                }
            }
            blockEnd = getBlockEnd(blockEnd);
            if (blockEnd.isValid()) {
                // Go to the line after the end of the current input block
                blockEnd = blockEnd.next();

                // Check for the library tag here
                library = getInputValue("library", blockEnd, foundTag);
            }
        }
    }

    // If we got the library tag, we can directly look up this input block structure
    if (library.size() > 0) {
        shared_ptr<EGS_BlockInput> inputBlock = inputStruct->getLibraryBlock(blockTitle.toStdString(), library.toStdString());
        if (inputBlock) {
#ifdef EDITOR_DEBUG
            egsInformation("EGS_Editor::getBlockInput: Found library: %s\n", library.toLatin1().data());
//             vector<shared_ptr<EGS_SingleInput>> singleInputs = inputBlock->getSingleInputs();
//             for (auto &inp : singleInputs) {
//                 const vector<string> vals = inp->getValues();
//                 egsInformation("   single %s\n", inp->getTag().c_str());
//                 for (auto&& val : vals) {
//                     egsInformation("      %s\n", val.c_str());
//                 }
//             }
//             vector<shared_ptr<EGS_BlockInput>> inputBlocks = inputBlock->getBlockInputs();
//             for (auto &block : inputBlocks) {
//                 singleInputs = inputBlock->getSingleInputs();
//                 for (auto &inp : singleInputs) {
//                     const vector<string> vals = inp->getValues();
//                     egsInformation("   single %s\n", inp->getTag().c_str());
//                     for (auto&& val : vals) {
//                         egsInformation("      %s\n", val.c_str());
//                     }
//                 }
//             }
#endif
            return inputBlock;
        }
    }

    // If we didn't get the library tag, we might be in a top-level block
    // like a geometry definition. Just return the block with the matching title
    shared_ptr<EGS_BlockInput> inputBlock = inputStruct->getBlockInput(blockTitle.toStdString());

#ifdef EDITOR_DEBUG
    egsInformation("EGS_Editor::getBlockInput: No library found, assuming '%s' is top-level block\n", blockTitle.toLatin1().data());
#endif

    return inputBlock;
}

QString EGS_Editor::getBlockTitle(QTextCursor cursor) {
    if (cursor == QTextCursor()) {
        cursor = textCursor();
    }

    vector<QString> innerList;
    QString blockTitle;
    bool withinOtherBlock = false;

    // Starting at the current line, starting iterating in reverse through
    // the previous lines
    for (QTextBlock block = cursor.block(); block.isValid(); block = block.previous()) {
        QString line = block.text().simplified();

        // Get block title
        int pos = line.lastIndexOf(":start ");
        if (pos >= 0) {
            pos += 7;
            int endPos = line.indexOf(":",pos);
            if (endPos > 0) {
                blockTitle = line.mid(pos, endPos-pos);
                if (innerList.size() > 0 && blockTitle == innerList.back()) {
                    innerList.pop_back();
                    blockTitle.clear();
                    withinOtherBlock = false;
                }
                else {
                    break;
                }
            }
        }

        // Save a vector of blocks that have already been closed
        // This means both a matching :start and :stop are above the cursor
        // so we're not inside the block
        pos = line.lastIndexOf(":stop ");
        if (pos >= 0) {
            pos += 6;
            int endPos = line.indexOf(":",pos);
            if (endPos > 0) {
                QString stopTitle = line.mid(pos, endPos-pos);
                innerList.push_back(stopTitle);
                withinOtherBlock = true;
            }
        }
    }

    return blockTitle;
}

QString EGS_Editor::getInputValue(QString inp, QTextBlock currentBlock, bool &foundTag) {
    QString value;
    vector<QString> innerList;
    bool withinOtherBlock = false;
    foundTag = false;

    // Get the last textblock in this input block
    // so that we search all the inputs in the block
    QTextBlock blockEnd = getBlockEnd(currentBlock);
    if (!blockEnd.isValid()) {
        return "";
    }

    // Starting at the last line, start iterating in reverse through
    // the previous lines
    blockEnd = blockEnd.previous();
    for (QTextBlock block = blockEnd; block.isValid(); block = block.previous()) {
        QString line = block.text().simplified();

        // Get block library for input blocks based on a shared library
        // e.g. geometries and sources
        // Only look for the library tag if we're not in a sub-block
        int pos;
        if (!withinOtherBlock) {
            pos = line.lastIndexOf(inp);
            if (pos >= 0) {
                int pos2 = line.lastIndexOf("=");
                if (pos2 > pos) {
                    QString tag = line.left(pos2).simplified();
                    if (egsEquivStr(tag.toStdString(), inp.simplified().toStdString())) {
                        foundTag = true;
                        value = line.right(line.size()-pos2-1).simplified();
                        break;
                    }
                }
            }
        }

        // Get block title
        pos = line.lastIndexOf(":start ");
        if (pos >= 0) {
            pos += 7;
            int endPos = line.indexOf(":",pos);
            if (endPos > 0) {
                QString blockTitle = line.mid(pos, endPos-pos);
                if (innerList.size() > 0 && blockTitle == innerList.back()) {
                    innerList.pop_back();
                    if (innerList.size() == 0) {
                        withinOtherBlock = false;
                    }
                }
                else {
                    // If we got to the start of the block,
                    // then we failed to find the input
                    return "";
                }
            }
        }

        // Save a vector of blocks that have already been closed
        // This means both a matching :start and :stop are above the cursor
        // so we're not inside the block
        pos = line.lastIndexOf(":stop ");
        if (pos >= 0) {
            pos += 6;
            int endPos = line.indexOf(":",pos);
            if (endPos > 0) {
                QString stopTitle = line.mid(pos, endPos-pos);
                innerList.push_back(stopTitle);
                withinOtherBlock = true;
            }
        }
    }

    return value;
}

QTextBlock EGS_Editor::getBlockEnd(QTextBlock currentBlock) {
    vector<QString> innerList;
    bool withinOtherBlock = false;

    // Starting at the current line, starting iterating in forward through
    // the next lines
    for (QTextBlock block = currentBlock; block.isValid(); block = block.next()) {
        QString line = block.text().simplified();

        // Save a vector of blocks that are contained within this input block
        // This means both a matching :start and :stop are below the cursor
        // so we're not inside the block
        int pos = line.lastIndexOf(":start ");
        if (pos >= 0) {
            pos += 7;
            int endPos = line.indexOf(":",pos);
            if (endPos > 0) {
                QString startTitle = line.mid(pos, endPos-pos);
                innerList.push_back(startTitle);
                withinOtherBlock = true;
            }
        }

        // Save a vector of blocks that are contained within this input block
        // so that we can skip them
        pos = line.lastIndexOf(":stop ");
        if (pos >= 0) {
            pos += 6;
            int startPos = line.indexOf(":",pos);
            if (startPos > 0) {
                QString blockTitle = line.mid(pos, startPos-pos);
                if (innerList.size() > 0 && blockTitle == innerList.back()) {
                    innerList.pop_back();
                    blockTitle.clear();
                    withinOtherBlock = false;
                }
                else {
                    return block;
                }
            }
        }
    }

    return QTextBlock();
}

bool EGS_Editor::inputHasDependency(shared_ptr<EGS_SingleInput> inp) {
    auto dependencyInp = inp->getDependencyInp();
    auto dependencyBlock = inp->getDependencyBlock();
    if (dependencyInp.size() < 1 && !dependencyBlock) {
        return false;
    }
    else {
        return true;
    }
}

bool EGS_Editor::inputDependencySatisfied(shared_ptr<EGS_SingleInput> inp, QTextCursor cursor) {
    if (cursor == QTextCursor()) {
        cursor = textCursor();
    }
    bool satisfied = true;

    // This is a list of inputs that the current input depends on
    auto dependencyInp = inp->getDependencyInp();

    // This is a list of values, that each of the dependencies above must match
    // These are like the required input parameters that we are checking against
    auto dependencyVal = inp->getDependencyVal();
    auto dependencyAnti = inp->getDependencyAnti();

    // Loop through the dependencies
    vector<bool> previousSatisfied;
    string previousTag;
    for (size_t i = 0; i < dependencyInp.size(); ++i) {
        if (!satisfied) {
            break;
        }

        string depTag = dependencyInp[i]->getTag();

        // Get the value from the input file
        bool foundTag;
        QString val = getInputValue(QString::fromStdString(depTag), cursor.block(), foundTag);

        if (foundTag && !dependencyAnti[i]) {
            if (dependencyVal[i].size() > 0) {
                if (egsEquivStr(val.toLatin1().data(), dependencyVal[i])) {
                    satisfied = true;
                }
                else {
                    satisfied = false;
                }
            }
            else {
                satisfied = true;
            }
        }
        else {
            // If this is an anti dependency, then we didn't want to find the tag
            // Note that we don't check the value, only whether or not the input tag is used
            if (!foundTag && dependencyAnti[i]) {
                satisfied = true;
            }
            else {
                satisfied = false;
            }
        }
        // Look ahead, if the following inputs have the same tag as this one (i)
        for (size_t j = i+1; j < dependencyInp.size(); ++j) {
            if (egsEquivStr(dependencyInp[j]->getTag(), depTag)) {
                // If we already were satisfied by the first one, just skip
                // ahead.
                // This is because dependencies with the same tag are treated
                // with an OR operation
                if (satisfied) {
                    // If we hit the end because all the tags matched, reset i
                    if (j == dependencyInp.size()-1) {
                        i = j;
                    }
                    continue;
                }
                else {
                    if (egsEquivStr(val.toLatin1().data(), dependencyVal[j])) {
                        satisfied = true;
                        // If we hit the end because all the tags matched, reset i
                        if (j == dependencyInp.size()-1) {
                            i = j;
                        }
                        continue;
                    }
                }
            }
            else {
                i = j-1;
                break;
            }
        }
    }

    // Check for any input blocks that this input depends on
    // We are doing an AND between the input-type dependencies and the block-type ones
    // So we can skip this section if the input-type dependencies failed
    if (satisfied) {
        auto dependencyBlock = inp->getDependencyBlock();

        if (dependencyBlock) {
            auto dependencyBlockAnti = inp->getDependencyBlockAnti();

            QTextBlock depBlock = findSiblingBlock(QString::fromStdString(dependencyBlock->getTitle()), cursor.block());

            if (depBlock.isValid()) {
                if (dependencyBlockAnti) {
                    satisfied = false;
                }
                else {
                    satisfied = true;
                }
            }
            else {
                if (dependencyBlockAnti) {
                    satisfied = true;
                }
                else {
                    satisfied = false;
                }
            }
        }
    }

    return satisfied;
}

QTextBlock EGS_Editor::findSiblingBlock(QString title, QTextBlock currentBlock) {
    vector<QString> innerList;
    bool withinOtherBlock = false;

    // Get the last textblock in this input block
    // so that we search all the inputs in the block
    QTextBlock blockEnd = getBlockEnd(currentBlock);
    if (!blockEnd.isValid()) {
        return QTextBlock();
    }

    // Starting at the last line, start iterating in reverse through
    // the previous lines
    blockEnd = blockEnd.previous();
    for (QTextBlock block = blockEnd; block.isValid(); block = block.previous()) {
        QString line = block.text().simplified();

        // Find a sibling block with the title we're looking for
        // Here we expect to be within another block because the start line counts as inside the block
        int pos;
        if (withinOtherBlock) {
            pos = line.lastIndexOf(":start " + title + ":");
            if (pos >= 0) {
                return block;
            }
        }

        // Get block title
        pos = line.lastIndexOf(":start ");
        if (pos >= 0) {
            pos += 7;
            int endPos = line.indexOf(":",pos);
            if (endPos > 0) {
                QString blockTitle = line.mid(pos, endPos-pos);
                if (innerList.size() > 0 && blockTitle == innerList.back()) {
                    innerList.pop_back();
                    if (innerList.size() == 0) {
                        withinOtherBlock = false;
                    }
                }
                else {
                    // If we got to the start of the current block,
                    // then we failed to find the target block
                    return QTextBlock();
                }
            }
        }

        // Save a vector of blocks that have already been closed
        // This means both a matching :start and :stop are above the cursor
        // so we're not inside the block
        pos = line.lastIndexOf(":stop ");
        if (pos >= 0) {
            pos += 6;
            int endPos = line.indexOf(":",pos);
            if (endPos > 0) {
                QString stopTitle = line.mid(pos, endPos-pos);
                innerList.push_back(stopTitle);
                withinOtherBlock = true;
            }
        }
    }

    return QTextBlock();
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
    else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // Insert 4 spaces instead of tabs
        if (keyEvent->key() == Qt::Key_Tab) {
            if (!popup->isVisible()) {
                insertPlainText("    ");
                return true;
            }
        }
        else if (keyEvent->key() == Qt::Key_Backtab) {
            // Delete 4 spaces from the front of the line
            QTextCursor cursor = textCursor();
            QString line = cursor.block().text();
            if (line.startsWith("    ")) {
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
                cursor.removeSelectedText();
            }
            else if (line.startsWith("\t")) {
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 1);
                cursor.removeSelectedText();
            }
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Return) {
            if (!popup->isVisible()) {

                QTextCursor cursor = textCursor();
                QString line = cursor.block().text();

                // Get the current indentation amount
                QString indentation;
                for (size_t i = 0; i < line.size(); ++i) {
                    if (line.at(i) == ' ') {
                        indentation += ' ';
                    }
                    else if (line.at(i) == '\t') {
                        indentation += "    ";
                    }
                    else {
                        break;
                    }
                }

                QString stopLine;
                int pos = line.lastIndexOf(":start ");
                int posInBlock = cursor.positionInBlock();
                if (pos > -1 && posInBlock > pos) {
                    stopLine = line.replace(pos, 7, ":stop ");
                }

                // If we inserted the ":stop" line, then also insert a line between
                // and leave the cursor there
                if (stopLine.size() > 0) {
                    insertPlainText("\n" + indentation + "    ");
                    insertPlainText("\n" + stopLine);
                    cursor.movePosition(QTextCursor::PreviousBlock);
                    cursor.movePosition(QTextCursor::EndOfBlock);
                    setTextCursor(cursor);

                    // Normally, we just insert a new line with matching indentation
                }
                else {
                    insertPlainText("\n" + indentation);
                }

                // Skip the usual return event! So we have to handle it here
                return true;
            }
        }
        else if (keyEvent->key() == Qt::Key_Escape) {
            popup->hide();
            popup->QWidget::releaseKeyboard();
        }
        else if (keyEvent->key() == Qt::Key_Right) {
            if (popup->isVisible()) {
                popupGrabbing = true;
                popup->QWidget::grabKeyboard();
                return true;
            }
        }
//     } else if(event->type() == QEvent::FocusOut || event->type() == QEvent::Move || event->type() == QEvent::Resize || event->type() == QEvent::Scroll || event->type() == QEvent::WindowDeactivate) {

        //} else if(event->type() == QEvent::Wheel || event->type() == QEvent::WindowDeactivate) {
    }
    else if (event->type() == QEvent::Wheel || event->type() == QEvent::FocusOut) {
        if (!popupGrabbing) {
            popup->hide();
            popup->QWidget::releaseKeyboard();
        }
    }
    else if (obj == popup && event->type() == QEvent::FocusIn) {
        popupGrabbing = false;
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

