#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>

namespace Ui { class SearchDialog; }

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr, QPlainTextEdit *textEdit = nullptr);
    ~SearchDialog();

private slots:
    void on_btnFindNext_clicked();
    void on_btnClose_clicked();

private:
    // 按当前选项构造查找标志
    QTextDocument::FindFlags findFlags() const;

    Ui::SearchDialog *ui;
    QPlainTextEdit *pTextEdit;   // 主窗口编辑控件
};

#endif // SEARCHDIALOG_H
