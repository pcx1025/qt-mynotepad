#include "replacedialog.h"
#include "ui_replacedialog.h"

#include <QMessageBox>
#include <QTextDocument>

ReplaceDialog::ReplaceDialog(QWidget *parent, QPlainTextEdit *textEdit)
    : QDialog(parent)
    , ui(new Ui::ReplaceDialog)
    , pTextEdit(textEdit)
{
    ui->setupUi(this);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

// 查找选项（“全部替换”固定向前查找，方向选项只影响逐个查找）
QTextDocument::FindFlags ReplaceDialog::findFlags() const
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

// 查找下一个（与查找对话框逻辑一致，支持连续查找）
void ReplaceDialog::on_btnFindNext_clicked()
{
    if (!pTextEdit)
        return;

    const QString target = ui->edtTarget->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请输入查找内容。"));
        return;
    }

    QTextDocument *doc = pTextEdit->document();
    QTextCursor from = pTextEdit->textCursor();
    if (from.hasSelection()) {
        from.setPosition(ui->radioDown->isChecked() ? from.selectionEnd()
                                                    : from.selectionStart());
    }

    const QTextCursor found = doc->find(target, from, findFlags());
    if (found.isNull()) {
        QMessageBox::information(this, QStringLiteral("提示"),
            QStringLiteral("未找到 \"%1\"。").arg(target));
    } else {
        pTextEdit->setTextCursor(found);
    }
}

// 替换：若当前选中的就是要替换的内容，替换之并自动查找下一处；
// 否则先定位到下一个匹配处
void ReplaceDialog::on_btnReplace_clicked()
{
    if (!pTextEdit)
        return;

    const QString target = ui->edtTarget->text();
    const QString toText = ui->edtToText->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请输入查找内容。"));
        return;
    }

    QTextCursor c = pTextEdit->textCursor();
    const Qt::CaseSensitivity cs = ui->chkCaseSensitive->isChecked()
                                       ? Qt::CaseSensitive : Qt::CaseInsensitive;

    if (c.hasSelection() &&
        QString::compare(c.selectedText(), target, cs) == 0) {
        c.insertText(toText);       // 替换当前选区（可撤销）
        on_btnFindNext_clicked();   // 自动定位下一处
    } else {
        // 先定位，再由用户决定是否替换
        on_btnFindNext_clicked();
    }
}

// 全部替换：使用编辑块保证一次撤销即可全部还原
void ReplaceDialog::on_btnReplaceAll_clicked()
{
    if (!pTextEdit)
        return;

    const QString target = ui->edtTarget->text();
    const QString toText = ui->edtToText->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"),
                                 QStringLiteral("请输入查找内容。"));
        return;
    }

    // 全部替换固定从文档开头向前查找，不受“向上/向下”影响
    QTextDocument::FindFlags flags;
    if (ui->chkCaseSensitive->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->chkWholeWord->isChecked())
        flags |= QTextDocument::FindWholeWords;

    QTextDocument *doc = pTextEdit->document();
    int count = 0;

    QTextCursor guard(doc);          // 编辑块：一次撤销恢复全部替换
    guard.beginEditBlock();
    QTextCursor f = doc->find(target, 0, flags);
    while (!f.isNull()) {
        f.insertText(toText);        // insertText 后光标位于替换文本末尾，继续向后找
        ++count;
        f = doc->find(target, f, flags);
    }
    guard.endEditBlock();

    if (count == 0) {
        // 边界：查找内容不存在
        QMessageBox::information(this, QStringLiteral("提示"),
            QStringLiteral("未找到 \"%1\"。").arg(target));
    } else {
        pTextEdit->document()->setModified(true);
        QMessageBox::information(this, QStringLiteral("替换完成"),
            QStringLiteral("共替换了 %1 处。").arg(count));
    }
}

void ReplaceDialog::on_btnClose_clicked()
{
    close();
}
