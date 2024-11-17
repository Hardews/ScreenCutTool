#ifndef SCREEN_WIDGET_H
#define SCREEN_WIDGET_H

#include <QWidget>
#include <QMenu>
#include <QPoint>
#include <QSize>
#include <QMouseEvent>

//截屏对象类
class Screen
{
public:
    enum STATUS {CLICK, MOV};
    Screen() {}
    Screen(QSize size);

    void setStart(QPoint pos);
    void setEnd(QPoint pos);
    QPoint getStart();
    QPoint getEnd();

    QPoint getLeftUp();
    QPoint getRightDown();

    STATUS getStatus();
    void setStatus(STATUS status);

    int width();
    int height();
    bool isInArea(QPoint pos);          // 检测 pos 是否在截图区域内
    void move(QPoint p);                // 按 p 移动截图区域

private:
    QPoint leftUpPos, rightDownPos;     // 截图区域 左上角、右下角 位置
    QPoint startPos, endPos;            //记录 鼠标开始位置、结束位置
    int maxWidth, maxHeight;            //记录屏幕大小
    STATUS status;                      //两个状态: 选择区域、移动区域

    void cmpPoint(QPoint &s, QPoint &e);//比较两位置，判断左上角、右下角
};

// 截屏窗口类
class ScreenWidget : public QWidget
{
    Q_OBJECT
public:
    static ScreenWidget *Instance(bool need_auto_copy = false, bool need_auto_save_as = false);
    explicit ScreenWidget(QWidget *parent = nullptr, bool need_auto_copy = false, bool need_auto_save_as = false);

private:
    static QScopedPointer<ScreenWidget> self;
    QMenu *menu;            // 右键菜单对象
    Screen *screen;         // 截屏对象
    QPixmap *fullScreen;    // 保存全屏图像
    QPixmap *bgScreen;      // 模糊背景图
    QColor bgColor;          // 背景颜色
    QPoint movPos;          // 坐标
    qreal pixelRatio;       // 缩放因子
    bool need_auto_copy;    // 是否需要自动复制
    bool need_auto_save_as; // 是否需要自动打开另存为
    QWidget *buttonContainer; // 截图完成后的底部栏

protected:
    void contextMenuEvent(QContextMenuEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void showEvent(QShowEvent *);
    void keyPressEvent(QKeyEvent *e);

private slots:
    void saveScreen();
    void saveFullScreen();
    void saveScreenOther();
    void saveFullOther();
    void saveScreenToClip(QPixmap screen);

private:
    void paintOperatorMenu();
};

#endif // SCREENWIDGET_H
