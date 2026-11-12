#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
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

    // ---- 文本变化/光标移动 → 状态栏信息、窗口标题（修改状态）----
    connect(ui->textEdit, &QPlainTextEdit::textChanged, this, &MainWindow::updateStatusLabels);
    connect(ui->textEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusLabels);
    connect(ui->textEdit, &QPlainTextEdit::modificationChanged, this, [this]{
        updateWindowTitle();
    });

    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ======================================================================
// 文件操作
// ======================================================================
void MainWindow::on_actionNew_triggered()
{
    if (!maybeSave())
        return;
    ui->textEdit->clear();
    ui->textEdit->document()->setModified(false);
    filePath.clear();
    updateWindowTitle();
    ui->statusbar->showMessage(QStringLiteral("新建文本文件"), 3000);
}

void MainWindow::on_actionOpen_triggered()
{
    if (!maybeSave())
        return;

    QString filename = QFileDialog::getOpenFileName(
        this, QStringLiteral("打开文件"), QString(),
        QStringLiteral("文本文件 (*.txt);;所有文件 (*.*)"));
    if (filename.isEmpty())
        return;                       // 用户取消

    QFile file(filename);
    if (!file.exists()) {             // 边界：文件不存在
        QMessageBox::warning(this, QStringLiteral("错误"),
            QStringLiteral("文件不存在：\n%1").arg(filename));
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {  // 边界：打开失败（无权限等）
        QMessageBox::warning(this, QStringLiteral("错误"),
            QStringLiteral("无法打开文件：\n%1\n%2").arg(filename, file.errorString()));
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    const QString content = in.readAll();
    if (in.status() == QTextStream::ReadCorruptData) {        // 边界：读取异常
        QMessageBox::warning(this, QStringLiteral("错误"),
            QStringLiteral("文件读取异常：\n%1").arg(filename));
        return;
    }

    ui->textEdit->setPlainText(content);
    ui->textEdit->document()->setModified(false);
    filePath = filename;
    updateWindowTitle();
    ui->statusbar->showMessage(QStringLiteral("已打开 %1").arg(filename), 3000);
}

bool MainWindow::saveFile()
{
    if (filePath.isEmpty())
        return saveFileAs();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("错误"),
            QStringLiteral("无法保存文件：\n%1\n%2").arg(filePath, file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << ui->textEdit->toPlainText();
    file.close();

    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
    ui->statusbar->showMessage(QStringLiteral("已保存 %1").arg(filePath), 3000);
    return true;
}

bool MainWindow::saveFileAs()
{
    QString filename = QFileDialog::getSaveFileName(
        this, QStringLiteral("另存为"), QString(),
        QStringLiteral("文本文件 (*.txt);;所有文件 (*.*)"));
    if (filename.isEmpty())
        return false;                  // 用户取消

    filePath = filename;
    return saveFile();
}

void MainWindow::on_actionSave_triggered()   { saveFile(); }
void MainWindow::on_actionSaveAs_triggered() { saveFileAs(); }
void MainWindow::on_actionExit_triggered()   { close(); }

// 关闭前：有未保存修改时提示 保存/不保存/取消
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

bool MainWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified())
        return true;

    const auto ret = QMessageBox::warning(this, QStringLiteral("MyNotepad"),
        QStringLiteral("文档已修改，是否保存修改？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:    return saveFile();
    case QMessageBox::Cancel:  return false;
    default:                   return true;   // Discard：放弃修改
    }
}

// 标题 = 文件名/未命名 + 修改标记 + 程序名
void MainWindow::updateWindowTitle()
{
    const QString name = filePath.isEmpty()
                             ? QStringLiteral("未命名")
                             : QFileInfo(filePath).fileName();
    const QString star = ui->textEdit->document()->isModified() ? "*" : "";
    setWindowTitle(QStringLiteral("%1%2 - MyNotepad").arg(name, star));
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

// 编辑/格式等功能将在后续提交中实现
void MainWindow::on_actionUndo_triggered()     {}
void MainWindow::on_actionRedo_triggered()     {}
void MainWindow::on_actionCut_triggered()      {}
void MainWindow::on_actionCopy_triggered()     {}
void MainWindow::on_actionPaste_triggered()    {}
void MainWindow::on_actionSelectAll_triggered(){}
void MainWindow::on_actionFont_triggered()     {}
void MainWindow::on_actionFontColor_triggered()        {}
void MainWindow::on_actionBackgroundColor_triggered()  {}
void MainWindow::on_actionLineWrap_triggered(bool)     {}
void MainWindow::on_actionShowToolbar_triggered(bool checked) { ui->toolBar->setVisible(checked); }
void MainWindow::on_actionShowStatusbar_triggered(bool checked) { ui->statusbar->setVisible(checked); }
void MainWindow::on_actionShowLineNumbers_triggered(bool) {}
void MainWindow::on_actionSearch_triggered()  {}
void MainWindow::on_actionReplace_triggered() {}
void MainWindow::on_actionAutoSave_triggered(bool) {}
void MainWindow::on_actionAbout_triggered()   {}
void MainWindow::on_actionAboutQt_triggered() {}
