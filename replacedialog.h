#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>

namespace Ui { class ReplaceDialog; }

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = nullptr, QPlainTextEdit *textEdit = nullptr);
    ~ReplaceDialog();

private slots:
    void on_btnFindNext_clicked();
    void on_btnReplace_clicked();       // 替换当前找到的一处，并自动查找下一处
    void on_btnReplaceAll_clicked();    // 全部替换
    void on_btnClose_clicked();

private:
    QTextDocument::FindFlags findFlags() const;   // 查找选项（不含方向）

    Ui::ReplaceDialog *ui;
    QPlainTextEdit *pTextEdit;
};

#endif // REPLACEDIALOG_H
