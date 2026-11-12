#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>

class CodeEditor;
class AboutDialog;
class SearchDialog;
class ReplaceDialog;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // ---- 自动化测试/截图辅助（不影响正常功能）----
    void loadSampleText();
    void applyDemoStyle();
    void setDemoModified();
    void setDemoToolbarVisible(bool v);
    void setDemoStatusbarVisible(bool v);
    void setDemoLineNumbers(bool v);
    void setDemoWrap(bool v);
    void selectDemoText();
    void demoShowDialog(const QString &name);

protected:
    // 关闭窗口前检查未保存修改
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();
    // 编辑
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSelectAll_triggered();
    // 格式
    void on_actionFont_triggered();
    void on_actionFontColor_triggered();
    void on_actionBackgroundColor_triggered();
    void on_actionLineWrap_triggered(bool checked);
    // 视图
    void on_actionShowToolbar_triggered(bool checked);
    void on_actionShowStatusbar_triggered(bool checked);
    void on_actionShowLineNumbers_triggered(bool checked);
    // 工具
    void on_actionSearch_triggered();
    void on_actionReplace_triggered();
    void on_actionAutoSave_triggered(bool checked);
    // 帮助
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();

private:
    // ---- 内部辅助 ----
    void updateWindowTitle();          // 根据文件路径与修改状态更新标题
    bool maybeSave();                  // 未保存时提示
    bool saveFile();                   // 保存（无路径时转另存为）
    bool saveFileAs();                 // 另存为
    void applyEditorStyle();           // 应用字体颜色/背景色
    void updateStatusLabels();         // 更新状态栏长度/行数/行列

    Ui::MainWindow *ui;
    CodeEditor *textEdit;              // 提升后的自定义编辑控件

    // 状态栏标签
    QLabel *statusLabel = nullptr;      // 长度/行数
    QLabel *statusCursorLabel = nullptr;// 行号/列号
    QLabel *authorLabel = nullptr;      // 作者

    // 查找/替换/关于对话框（懒创建）
    SearchDialog *searchDialog = nullptr;
    ReplaceDialog *replaceDialog = nullptr;

    // 字体颜色与背景色（持久化，避免互相覆盖）
    QString m_textColor = "#000000";
    QString m_bgColor   = "#ffffff";

    // 定时自动保存
    QTimer autoSaveTimer;

    QString filePath;                  // 当前文件路径，空表示未保存过
};

#endif // MAINWINDOW_H
