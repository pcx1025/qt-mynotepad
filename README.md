# MyNotepad（实验2：简单文本编辑器）

东莞理工学院《Qt应用程序开发》实验2作业。基于 QPlainTextEdit 的多窗口文本编辑器，
界面由 UI 设计器搭建，菜单/工具栏使用 Action 编辑器按功能分组设计。

## 功能

- 文件：新建、打开、保存、另存为；未保存关闭提示；异常文件（不存在/无权限/读取失败）处理
- 编辑：撤销、重做、剪切、复制、粘贴、全选，Action 使能状态与编辑状态联动
- 格式：字体、字体颜色、背景色设置，自动换行开关
- 视图：工具栏/状态栏/行号 显示与隐藏
- 工具：查找（区分大小写/全字匹配/向上向下/连续查找）、替换（逐个替换/全部替换）、定时自动保存（选做）
- 状态栏：长度/行数、光标行号/列号实时更新
- 自定义 CodeEditor 类（QPlainTextEdit 派生，参考 Qt Code Editor Example），
  在 mainwindow.ui 中通过 promote 提升替换标准控件，支持行号显示

## 界面结构

- 主窗口：mainwindow.ui（6 个菜单：文件/编辑/格式/视图/工具/帮助 + 工具栏 + 状态栏）
- 关于对话框：aboutdialog.ui
- 查找对话框：searchdialog.ui（非模态，支持连续查找）
- 替换对话框：replacedialog.ui（替换/全部替换，编辑块保证一次撤销还原）

## 编译

Qt 5.12+ / Qt 6 均可（Qt 5.15.2 + MinGW 8.1 测试通过）：

```
qmake MyNotepad.pro
mingw32-make release
```
