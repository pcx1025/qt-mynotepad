#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codeeditor.h"
#include "aboutdialog.h"
#include "searchdialog.h"
#include "replacedialog.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QFontDialog>
#include <QColorDialog>
#include <QStyle>
#include <QClipboard>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    textEdit = qobject_cast<CodeEditor *>(ui->textEdit);

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
    connect(textEdit, &QPlainTextEdit::textChanged, this, &MainWindow::updateStatusLabels);
    connect(textEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusLabels);
    connect(textEdit, &QPlainTextEdit::modificationChanged, this, [this]{
        updateWindowTitle();
    });

    // ---- 编辑类 Action 的使能状态与编辑状态保持一致 ----
    connect(textEdit, &QPlainTextEdit::copyAvailable,
            ui->actionCut, &QAction::setEnabled);           // 有选中才可剪切/复制
    connect(textEdit, &QPlainTextEdit::copyAvailable,
            ui->actionCopy, &QAction::setEnabled);
    connect(textEdit, &QPlainTextEdit::undoAvailable,
            ui->actionUndo, &QAction::setEnabled);
    connect(textEdit, &QPlainTextEdit::redoAvailable,
            ui->actionRedo, &QAction::setEnabled);
    // 剪贴板有文本才可粘贴
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this]{
        const QString t = QApplication::clipboard()->text();
        ui->actionPaste->setEnabled(!t.isEmpty());
    });
    // 初始使能状态
    ui->actionCut->setEnabled(false);
    ui->actionCopy->setEnabled(false);
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());

    // ---- 定时自动保存（选做功能）：30 秒一次 ----
    autoSaveTimer.setInterval(30 * 1000);
    connect(&autoSaveTimer, &QTimer::timeout, this, [this]{
        if (ui->actionAutoSave->isChecked() && !filePath.isEmpty()
            && textEdit->document()->isModified()) {
            if (saveFile())
                ui->statusbar->showMessage(
                    QStringLiteral("已自动保存 %1")
                        .arg(QTime::currentTime().toString("HH:mm:ss")), 3000);
        }
    });
    autoSaveTimer.start();

    updateWindowTitle();
    applyEditorStyle();
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
    textEdit->clear();
    textEdit->setPlainText(QString());
    textEdit->document()->setModified(false);
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

    textEdit->setPlainText(content);
    textEdit->document()->setModified(false);
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
    out << textEdit->toPlainText();
    file.close();

    textEdit->document()->setModified(false);
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
    if (!textEdit->document()->isModified())
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
    const QString star = textEdit->document()->isModified() ? "*" : "";
    setWindowTitle(QStringLiteral("%1%2 - MyNotepad").arg(name, star));
}

// ======================================================================
// 编辑操作：直接复用 QPlainTextEdit 提供的槽
// ======================================================================
void MainWindow::on_actionUndo_triggered()     { textEdit->undo(); }
void MainWindow::on_actionRedo_triggered()     { textEdit->redo(); }
void MainWindow::on_actionCut_triggered()      { textEdit->cut(); }
void MainWindow::on_actionCopy_triggered()     { textEdit->copy(); }
void MainWindow::on_actionPaste_triggered()    { textEdit->paste(); }
void MainWindow::on_actionSelectAll_triggered(){ textEdit->selectAll(); }

// ======================================================================
// 格式：字体、字体颜色、背景色、自动换行
// ======================================================================
void MainWindow::on_actionFont_triggered()
{
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, textEdit->font(), this,
                                            QStringLiteral("选择字体"));
    if (ok)
        textEdit->setFont(font);
}

void MainWindow::on_actionFontColor_triggered()
{
    const QColor color = QColorDialog::getColor(QColor(m_textColor), this,
                                                QStringLiteral("选择字体颜色"));
    if (color.isValid()) {
        m_textColor = color.name();
        applyEditorStyle();
    }
}

void MainWindow::on_actionBackgroundColor_triggered()
{
    const QColor color = QColorDialog::getColor(QColor(m_bgColor), this,
                                                QStringLiteral("选择背景色"));
    if (color.isValid()) {
        m_bgColor = color.name();
        applyEditorStyle();
    }
}

void MainWindow::applyEditorStyle()
{
    // 同时维护字体颜色与背景色，避免互相覆盖
    textEdit->setStyleSheet(QStringLiteral(
        "QPlainTextEdit {"
        "  color: %1;"
        "  background-color: %2;"
        "  selection-background-color: #3399ff;"
        "}").arg(m_textColor, m_bgColor));
}

void MainWindow::on_actionLineWrap_triggered(bool checked)
{
    textEdit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth
                                      : QPlainTextEdit::NoWrap);
}

// ======================================================================
// 视图：工具栏/状态栏/行号 显示与隐藏
// ======================================================================
void MainWindow::on_actionShowToolbar_triggered(bool checked)
{
    ui->toolBar->setVisible(checked);
}

void MainWindow::on_actionShowStatusbar_triggered(bool checked)
{
    ui->statusbar->setVisible(checked);
}

void MainWindow::on_actionShowLineNumbers_triggered(bool checked)
{
    textEdit->setLineNumbersVisible(checked);
}

// ======================================================================
// 工具：查找、替换、自动保存开关
// ======================================================================
void MainWindow::on_actionSearch_triggered()
{
    if (!searchDialog)
        searchDialog = new SearchDialog(this, textEdit);
    searchDialog->show();
    searchDialog->raise();
    searchDialog->activateWindow();
}

void MainWindow::on_actionReplace_triggered()
{
    if (!replaceDialog)
        replaceDialog = new ReplaceDialog(this, textEdit);
    replaceDialog->show();
    replaceDialog->raise();
    replaceDialog->activateWindow();
}

void MainWindow::on_actionAutoSave_triggered(bool checked)
{
    if (filePath.isEmpty() && checked)
        QMessageBox::information(this, QStringLiteral("提示"),
            QStringLiteral("当前文档尚未保存过，自动保存将在首次保存后生效。"));
}

// ======================================================================
// 帮助
// ======================================================================
void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

// ======================================================================
// 状态栏信息更新
// ======================================================================
void MainWindow::updateStatusLabels()
{
    statusLabel->setText(QStringLiteral("长度: %1   行数: %2")
                             .arg(textEdit->toPlainText().size())
                             .arg(textEdit->blockCount()));
    const QTextCursor c = textEdit->textCursor();
    statusCursorLabel->setText(QStringLiteral("行: %1   列: %2")
                                   .arg(c.blockNumber() + 1)
                                   .arg(c.columnNumber() + 1));
}

// ======================================================================
// 自动化测试/截图辅助
// ======================================================================
void MainWindow::loadSampleText()
{
    textEdit->setPlainText(QStringLiteral(
        "Qt 是一个跨平台的 C++ 应用程序开发框架。\n"
        "MyNotepad 是 Qt应用程序开发课程的第二个实验：简单文本编辑器。\n"
        "The QPlainTextEdit widget provides a plain text editor.\n"
        "支持新建、打开、保存、另存为文件操作。\n"
        "支持撤销、重做、剪切、复制、粘贴、全选。\n"
        "支持字体、字体颜色、背景色设置，支持自动换行。\n"
        "支持行号显示与隐藏、工具栏与状态栏显示与隐藏。\n"
        "支持查找与替换、未保存关闭提示、定时自动保存。\n"
        "这是一条很长的文本行，用于演示自动换行功能：打开自动换行时它会在窗口边缘自动折行显示，关闭自动换行时则出现水平滚动条，需要拖动滚动条才能看到后面的内容。"));
    textEdit->document()->setModified(false);
    updateWindowTitle();
    updateStatusLabels();
}

void MainWindow::applyDemoStyle()
{
    textEdit->setFont(QFont("Microsoft YaHei UI", 14));
    m_textColor = "#1a56b0";
    m_bgColor = "#fffbe6";
    applyEditorStyle();
    ui->actionLineWrap->setChecked(true);
    on_actionLineWrap_triggered(true);
}

void MainWindow::setDemoModified()
{
    textEdit->document()->setModified(true);
    updateWindowTitle();
}

void MainWindow::setDemoToolbarVisible(bool v)
{
    ui->toolBar->setVisible(v);
    ui->actionShowToolbar->setChecked(v);
}

void MainWindow::setDemoStatusbarVisible(bool v)
{
    ui->statusbar->setVisible(v);
    ui->actionShowStatusbar->setChecked(v);
}

void MainWindow::setDemoLineNumbers(bool v)
{
    textEdit->setLineNumbersVisible(v);
    ui->actionShowLineNumbers->setChecked(v);
}

void MainWindow::setDemoWrap(bool v)
{
    ui->actionLineWrap->setChecked(v);
    on_actionLineWrap_triggered(v);
}

void MainWindow::selectDemoText()
{
    QTextCursor c = textEdit->textCursor();
    c.movePosition(QTextCursor::Start);
    c.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
    textEdit->setTextCursor(c);
}

void MainWindow::demoShowDialog(const QString &name)
{
    if (name == "about") {
        // 非模态展示以便截图（正常路径使用 exec() 模态对话框）
        AboutDialog *d = new AboutDialog(this);
        d->setAttribute(Qt::WA_DeleteOnClose);
        d->show();
    } else if (name == "find") {
        on_actionSearch_triggered();
    } else if (name == "replace") {
        on_actionReplace_triggered();
    }
}
