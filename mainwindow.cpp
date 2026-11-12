#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ---- 给 Action 设置标准图标（工具栏显示图标+文字）----
    QStyle *st = style();
    ui->actionNew->setIcon(st->standardIcon(QStyle::SP_FileIcon));
    ui->actionOpen->setIcon(st->standardIcon(QStyle::SP_DialogOpenButton));
    ui->actionSave->setIcon(st->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionUndo->setIcon(st->standardIcon(QStyle::SP_ArrowBack));
    ui->actionRedo->setIcon(st->standardIcon(QStyle::SP_ArrowForward));
    ui->actionCut->setIcon(st->standardIcon(QStyle::SP_TrashIcon));
    ui->actionCopy->setIcon(st->standardIcon(QStyle::SP_FileDialogDetailedView));
    ui->actionPaste->setIcon(st->standardIcon(QStyle::SP_DialogApplyButton));
    ui->actionSearch->setIcon(st->standardIcon(QStyle::SP_FileDialogContentsView));
    ui->actionReplace->setIcon(st->standardIcon(QStyle::SP_BrowserReload));

    // ---- 状态栏：长度/行数、行号/列号、作者 三个永久标签 ----
    statusLabel = new QLabel(this);
    statusLabel->setMinimumWidth(160);
    ui->statusbar->addPermanentWidget(statusLabel);

    statusCursorLabel = new QLabel(this);
    statusCursorLabel->setMinimumWidth(120);
    ui->statusbar->addPermanentWidget(statusCursorLabel);

    authorLabel = new QLabel(QStringLiteral("作者：肖志宇"), this);
    ui->statusbar->addPermanentWidget(authorLabel);
    updateStatusLabels();

    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 各功能将在后续提交中实现
void MainWindow::on_actionNew_triggered() {}
void MainWindow::on_actionOpen_triggered() {}
void MainWindow::on_actionSave_triggered() {}
void MainWindow::on_actionSaveAs_triggered() {}
void MainWindow::on_actionExit_triggered() { close(); }
void MainWindow::on_actionUndo_triggered() {}
void MainWindow::on_actionRedo_triggered() {}
void MainWindow::on_actionCut_triggered() {}
void MainWindow::on_actionCopy_triggered() {}
void MainWindow::on_actionPaste_triggered() {}
void MainWindow::on_actionSelectAll_triggered() {}
void MainWindow::on_actionFont_triggered() {}
void MainWindow::on_actionFontColor_triggered() {}
void MainWindow::on_actionBackgroundColor_triggered() {}
void MainWindow::on_actionLineWrap_triggered(bool) {}
void MainWindow::on_actionShowToolbar_triggered(bool checked) { ui->toolBar->setVisible(checked); }
void MainWindow::on_actionShowStatusbar_triggered(bool checked) { ui->statusbar->setVisible(checked); }
void MainWindow::on_actionShowLineNumbers_triggered(bool) {}
void MainWindow::on_actionSearch_triggered() {}
void MainWindow::on_actionReplace_triggered() {}
void MainWindow::on_actionAutoSave_triggered(bool) {}
void MainWindow::on_actionAbout_triggered() {}
void MainWindow::on_actionAboutQt_triggered() {}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(QStringLiteral("未命名 - MyNotepad"));
}

void MainWindow::updateStatusLabels()
{
    statusLabel->setText(QStringLiteral("长度: %1   行数: %2")
                             .arg(ui->textEdit->toPlainText().size())
                             .arg(ui->textEdit->blockCount()));
    const QTextCursor c = ui->textEdit->textCursor();
    statusCursorLabel->setText(QStringLiteral("行: %1   列: %2")
                                   .arg(c.blockNumber() + 1)
                                   .arg(c.columnNumber() + 1));
}
