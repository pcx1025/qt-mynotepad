#include "searchdialog.h"
#include "ui_searchdialog.h"

#include <QMessageBox>
#include <QTextDocument>

SearchDialog::SearchDialog(QWidget *parent, QPlainTextEdit *textEdit)
    : QDialog(parent)
    , ui(new Ui::SearchDialog)
    , pTextEdit(textEdit)
{
    ui->setupUi(this);
    // 由主窗口持有指针、重复 show() 复用，避免重复创建
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

QTextDocument::FindFlags SearchDialog::findFlags() const
{
    QTextDocument::FindFlags flags;
    if (ui->chkCaseSensitive->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->chkWholeWord->isChecked())
        flags |= QTextDocument::FindWholeWords;
    if (ui->radioUp->isChecked())
        flags |= QTextDocument::FindBackward;
    return flags;
}

// 查找下一个：支持连续查找（从当前选区之后继续）
void SearchDialog::on_btnFindNext_clicked()
{
    if (!pTextEdit)
        return;

    const QString target = ui->searchText->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请输入查找内容。"));
        return;
    }

    QTextDocument *doc = pTextEdit->document();
    QTextCursor from = pTextEdit->textCursor();

    // 连续查找：从当前选区的结束位置（向下）或起始位置（向上）继续
    if (from.hasSelection()) {
        from.setPosition(ui->radioDown->isChecked() ? from.selectionEnd()
                                                    : from.selectionStart());
    }

    const QTextCursor found = doc->find(target, from, findFlags());
    if (found.isNull()) {
        // 边界：查找内容不存在（已到文档末尾/开头）
        QMessageBox::information(this, QStringLiteral("提示"),
            QStringLiteral("未找到 \"%1\"。").arg(target));
    } else {
        // 选中找到的内容，便于用户确认
        pTextEdit->setTextCursor(found);
    }
}

void SearchDialog::on_btnClose_clicked()
{
    close();
}
