#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>

// 行号区域子控件：只负责把绘制事件转发给 CodeEditor
class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(class CodeEditor *editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *codeEditor;
};

// 参考 Qt 官方 Code Editor Example：从 QPlainTextEdit 派生，
// 支持行号显示/隐藏、当前行高亮
// 在 mainwindow.ui 中通过 promote（提升）替换标准 QPlainTextEdit
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;

    bool lineNumbersVisible() const { return m_lineNumbersVisible; }
    void setLineNumbersVisible(bool visible);   // 供“视图-显示行号”开关调用

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    LineNumberArea *lineNumberArea;
    bool m_lineNumbersVisible = true;
};

#endif // CODEEDITOR_H
