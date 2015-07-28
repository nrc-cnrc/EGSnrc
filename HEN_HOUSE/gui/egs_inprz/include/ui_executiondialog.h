/********************************************************************************
** Form generated from reading UI file 'executiondialog.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXECUTIONDIALOG_H
#define UI_EXECUTIONDIALOG_H

#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_MExecutionDialog
{
public:
    QVBoxLayout *vboxLayout;
    QVBoxLayout *vboxLayout1;
    QGroupBox *inputFileGroupBox;
    QHBoxLayout *hboxLayout;
    QLabel *inputFileLabel;
    QGroupBox *runModeButtonGroup;
    QHBoxLayout *hboxLayout1;
    QHBoxLayout *hboxLayout2;
    QRadioButton *interactiveRadioButton;
    QRadioButton *batchRadioButton;
    QGroupBox *batchGroupBox;
    QVBoxLayout *vboxLayout2;
    QVBoxLayout *vboxLayout3;
    QHBoxLayout *hboxLayout3;
    QSpinBox *NumJobSpinBox;
    QSpacerItem *Spacer26;
    QLabel *NumJobLabel;
    QHBoxLayout *hboxLayout4;
    QComboBox *queueSystemcomboBox;
    QSpacerItem *spacer43;
    QLabel *queueSystemLabel;
    QHBoxLayout *hboxLayout5;
    QComboBox *QueueComboBox;
    QSpacerItem *Spacer27;
    QLabel *QueueLabel;
    QFrame *Frame7;
    QHBoxLayout *hboxLayout6;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;

    void setupUi(QDialog *MExecutionDialog)
    {
        if (MExecutionDialog->objectName().isEmpty())
            MExecutionDialog->setObjectName(QString::fromUtf8("MExecutionDialog"));
        MExecutionDialog->resize(354, 375);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MExecutionDialog->sizePolicy().hasHeightForWidth());
        MExecutionDialog->setSizePolicy(sizePolicy);
        MExecutionDialog->setMinimumSize(QSize(270, 330));
        MExecutionDialog->setMaximumSize(QSize(1000, 375));
        MExecutionDialog->setSizeGripEnabled(false);
        vboxLayout = new QVBoxLayout(MExecutionDialog);
        vboxLayout->setSpacing(6);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout1 = new QVBoxLayout();
        vboxLayout1->setSpacing(6);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        inputFileGroupBox = new QGroupBox(MExecutionDialog);
        inputFileGroupBox->setObjectName(QString::fromUtf8("inputFileGroupBox"));
        sizePolicy.setHeightForWidth(inputFileGroupBox->sizePolicy().hasHeightForWidth());
        inputFileGroupBox->setSizePolicy(sizePolicy);
        hboxLayout = new QHBoxLayout(inputFileGroupBox);
        hboxLayout->setSpacing(6);
        hboxLayout->setContentsMargins(11, 11, 11, 11);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        inputFileLabel = new QLabel(inputFileGroupBox);
        inputFileLabel->setObjectName(QString::fromUtf8("inputFileLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(232);
        sizePolicy1.setVerticalStretch(232);
        sizePolicy1.setHeightForWidth(inputFileLabel->sizePolicy().hasHeightForWidth());
        inputFileLabel->setSizePolicy(sizePolicy1);
        inputFileLabel->setMinimumSize(QSize(210, 40));
        inputFileLabel->setAlignment(Qt::AlignVCenter);
        inputFileLabel->setWordWrap(true);

        hboxLayout->addWidget(inputFileLabel);


        vboxLayout1->addWidget(inputFileGroupBox);

        runModeButtonGroup = new QGroupBox(MExecutionDialog);
        runModeButtonGroup->setObjectName(QString::fromUtf8("runModeButtonGroup"));
        sizePolicy.setHeightForWidth(runModeButtonGroup->sizePolicy().hasHeightForWidth());
        runModeButtonGroup->setSizePolicy(sizePolicy);
        hboxLayout1 = new QHBoxLayout(runModeButtonGroup);
        hboxLayout1->setSpacing(6);
        hboxLayout1->setContentsMargins(11, 11, 11, 11);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setSpacing(6);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        interactiveRadioButton = new QRadioButton(runModeButtonGroup);
        interactiveRadioButton->setObjectName(QString::fromUtf8("interactiveRadioButton"));
        interactiveRadioButton->setChecked(true);

        hboxLayout2->addWidget(interactiveRadioButton);

        batchRadioButton = new QRadioButton(runModeButtonGroup);
        batchRadioButton->setObjectName(QString::fromUtf8("batchRadioButton"));

        hboxLayout2->addWidget(batchRadioButton);


        hboxLayout1->addLayout(hboxLayout2);


        vboxLayout1->addWidget(runModeButtonGroup);

        batchGroupBox = new QGroupBox(MExecutionDialog);
        batchGroupBox->setObjectName(QString::fromUtf8("batchGroupBox"));
        batchGroupBox->setEnabled(false);
        sizePolicy.setHeightForWidth(batchGroupBox->sizePolicy().hasHeightForWidth());
        batchGroupBox->setSizePolicy(sizePolicy);
        vboxLayout2 = new QVBoxLayout(batchGroupBox);
        vboxLayout2->setSpacing(6);
        vboxLayout2->setContentsMargins(11, 11, 11, 11);
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        vboxLayout3 = new QVBoxLayout();
        vboxLayout3->setSpacing(6);
        vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setSpacing(6);
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        NumJobSpinBox = new QSpinBox(batchGroupBox);
        NumJobSpinBox->setObjectName(QString::fromUtf8("NumJobSpinBox"));
        NumJobSpinBox->setMinimum(1);
        NumJobSpinBox->setMaximum(10000);
        NumJobSpinBox->setValue(1);

        hboxLayout3->addWidget(NumJobSpinBox);

        Spacer26 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout3->addItem(Spacer26);

        NumJobLabel = new QLabel(batchGroupBox);
        NumJobLabel->setObjectName(QString::fromUtf8("NumJobLabel"));
        NumJobLabel->setWordWrap(false);

        hboxLayout3->addWidget(NumJobLabel);


        vboxLayout3->addLayout(hboxLayout3);

        hboxLayout4 = new QHBoxLayout();
        hboxLayout4->setSpacing(6);
        hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
        queueSystemcomboBox = new QComboBox(batchGroupBox);
        queueSystemcomboBox->setObjectName(QString::fromUtf8("queueSystemcomboBox"));

        hboxLayout4->addWidget(queueSystemcomboBox);

        spacer43 = new QSpacerItem(30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout4->addItem(spacer43);

        queueSystemLabel = new QLabel(batchGroupBox);
        queueSystemLabel->setObjectName(QString::fromUtf8("queueSystemLabel"));
        queueSystemLabel->setWordWrap(false);

        hboxLayout4->addWidget(queueSystemLabel);


        vboxLayout3->addLayout(hboxLayout4);

        hboxLayout5 = new QHBoxLayout();
        hboxLayout5->setSpacing(6);
        hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
        QueueComboBox = new QComboBox(batchGroupBox);
        QueueComboBox->setObjectName(QString::fromUtf8("QueueComboBox"));

        hboxLayout5->addWidget(QueueComboBox);

        Spacer27 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout5->addItem(Spacer27);

        QueueLabel = new QLabel(batchGroupBox);
        QueueLabel->setObjectName(QString::fromUtf8("QueueLabel"));
        QueueLabel->setWordWrap(false);

        hboxLayout5->addWidget(QueueLabel);


        vboxLayout3->addLayout(hboxLayout5);


        vboxLayout2->addLayout(vboxLayout3);


        vboxLayout1->addWidget(batchGroupBox);

        Frame7 = new QFrame(MExecutionDialog);
        Frame7->setObjectName(QString::fromUtf8("Frame7"));
        sizePolicy.setHeightForWidth(Frame7->sizePolicy().hasHeightForWidth());
        Frame7->setSizePolicy(sizePolicy);
        Frame7->setFrameShape(QFrame::StyledPanel);
        Frame7->setFrameShadow(QFrame::Sunken);
        hboxLayout6 = new QHBoxLayout(Frame7);
        hboxLayout6->setSpacing(6);
        hboxLayout6->setContentsMargins(11, 11, 11, 11);
        hboxLayout6->setObjectName(QString::fromUtf8("hboxLayout6"));
        buttonOk = new QPushButton(Frame7);
        buttonOk->setObjectName(QString::fromUtf8("buttonOk"));
        buttonOk->setAutoDefault(true);
        buttonOk->setDefault(true);

        hboxLayout6->addWidget(buttonOk);

        buttonCancel = new QPushButton(Frame7);
        buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));
        buttonCancel->setAutoDefault(true);

        hboxLayout6->addWidget(buttonCancel);


        vboxLayout1->addWidget(Frame7);


        vboxLayout->addLayout(vboxLayout1);


        retranslateUi(MExecutionDialog);
        QObject::connect(buttonCancel, SIGNAL(clicked()), MExecutionDialog, SLOT(reject()));
        QObject::connect(batchRadioButton, SIGNAL(toggled(bool)), MExecutionDialog, SLOT(update_batch()));
        QObject::connect(buttonOk, SIGNAL(clicked()), MExecutionDialog, SLOT(run()));

        QMetaObject::connectSlotsByName(MExecutionDialog);
    } // setupUi

    void retranslateUi(QDialog *MExecutionDialog)
    {
        MExecutionDialog->setWindowTitle(QApplication::translate("MExecutionDialog", "Execution Setup", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        MExecutionDialog->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_WHATSTHIS
        MExecutionDialog->setWhatsThis(QApplication::translate("MExecutionDialog", "Define parameters for user code execution", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
#ifndef QT_NO_TOOLTIP
        inputFileGroupBox->setToolTip(QApplication::translate("MExecutionDialog", "Name of current input file", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_WHATSTHIS
        inputFileGroupBox->setWhatsThis(QApplication::translate("MExecutionDialog", "Name of current input file", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        inputFileGroupBox->setTitle(QApplication::translate("MExecutionDialog", "Input file name to run :", 0, QApplication::UnicodeUTF8));
        inputFileLabel->setText(QApplication::translate("MExecutionDialog", "input file name", 0, QApplication::UnicodeUTF8));
        runModeButtonGroup->setTitle(QApplication::translate("MExecutionDialog", "Execution mode", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        interactiveRadioButton->setToolTip(QApplication::translate("MExecutionDialog", "run program locally", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        interactiveRadioButton->setText(QApplication::translate("MExecutionDialog", "interactive", 0, QApplication::UnicodeUTF8));
        batchRadioButton->setText(QApplication::translate("MExecutionDialog", "batch", 0, QApplication::UnicodeUTF8));
        batchGroupBox->setTitle(QApplication::translate("MExecutionDialog", "Batch run", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        NumJobSpinBox->setToolTip(QApplication::translate("MExecutionDialog", "If > 1, jobs processed in parallel using pprocess", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        NumJobLabel->setToolTip(QApplication::translate("MExecutionDialog", "number of jobs to run", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        NumJobLabel->setText(QApplication::translate("MExecutionDialog", "# of jobs", 0, QApplication::UnicodeUTF8));
        queueSystemcomboBox->clear();
        queueSystemcomboBox->insertItems(0, QStringList()
         << QApplication::translate("MExecutionDialog", "at", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MExecutionDialog", "nqs", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MExecutionDialog", "pbs", 0, QApplication::UnicodeUTF8)
        );
        queueSystemLabel->setText(QApplication::translate("MExecutionDialog", "Queueing system", 0, QApplication::UnicodeUTF8));
        QueueComboBox->clear();
        QueueComboBox->insertItems(0, QStringList()
         << QApplication::translate("MExecutionDialog", "long", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MExecutionDialog", "medium", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MExecutionDialog", "short", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        QueueComboBox->setToolTip(QApplication::translate("MExecutionDialog", "type of queue", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        QueueLabel->setToolTip(QApplication::translate("MExecutionDialog", "type of queue", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        QueueLabel->setText(QApplication::translate("MExecutionDialog", "Queue", 0, QApplication::UnicodeUTF8));
        buttonOk->setText(QApplication::translate("MExecutionDialog", "&Run", 0, QApplication::UnicodeUTF8));
        buttonOk->setShortcut(QApplication::translate("MExecutionDialog", "Alt+R", 0, QApplication::UnicodeUTF8));
        buttonCancel->setText(QApplication::translate("MExecutionDialog", "&Close", 0, QApplication::UnicodeUTF8));
        buttonCancel->setShortcut(QApplication::translate("MExecutionDialog", "Alt+C", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MExecutionDialog: public Ui_MExecutionDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXECUTIONDIALOG_H
