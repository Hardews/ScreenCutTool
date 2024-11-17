#include "screen_widget.h"
#include <QMutex>
#include <QApplication>
#include <QPainter>
#include <QGuiApplication>
#include <QFileDialog>
#include <QEvent>
#include <QDateTime>
#include <QStringList>
#include <QClipboard>
#include <QPushButton>
#include <QHBoxLayout>

#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
#include <QScreen>
#endif

#define STRDATETIME qPrintable (QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss"))

Screen::Screen(QSize size)
{
    maxWidth = size.width();
    maxHeight = size.height();

    startPos = QPoint(-1, -1);
    endPos = startPos;
    leftUpPos = startPos;
    rightDownPos = startPos;
    status = CLICK;
}

int Screen::width()
{
    return maxWidth;
}

int Screen::height()
{
    return maxHeight;
}

Screen::STATUS Screen::getStatus()
{
    return status;
}

void Screen::setStatus(STATUS status)
{
    this->status = status;
}

void Screen::setEnd(QPoint pos)
{
    endPos = pos;
    leftUpPos = startPos;
    rightDownPos = endPos;
    cmpPoint(leftUpPos, rightDownPos);
}

void Screen::setStart(QPoint pos)
{
    startPos = pos;
}

QPoint Screen::getEnd()
{
    return endPos;
}

QPoint Screen::getStart()
{
    return startPos;
}

QPoint Screen::getLeftUp()
{
    return leftUpPos;
}

QPoint Screen::getRightDown()
{
    return rightDownPos;
}

bool Screen::isInArea(QPoint pos)
{
    if (pos.x() > leftUpPos.x() && pos.x() < rightDownPos.x() && pos.y() > leftUpPos.y() && pos.y() < rightDownPos.y()) {
        return true;
    }

    return false;
}

void Screen::move(QPoint p)
{
    int lx = leftUpPos.x() + p.x();
    int ly = leftUpPos.y() + p.y();
    int rx = rightDownPos.x() + p.x();
    int ry = rightDownPos.y() + p.y();

    if (lx < 0) {
        lx = 0;
        rx -= p.x();
    }

    if (ly < 0) {
        ly = 0;
        ry -= p.y();
    }

    if (rx > maxWidth)  {
        rx = maxWidth;
        lx -= p.x();
    }

    if (ry > maxHeight) {
        ry = maxHeight;
        ly -= p.y();
    }

    leftUpPos = QPoint(lx, ly);
    rightDownPos = QPoint(rx, ry);
    startPos = leftUpPos;
    endPos = rightDownPos;
}

void Screen::cmpPoint(QPoint &leftTop, QPoint &rightDown)
{
    QPoint l = leftTop;
    QPoint r = rightDown;

    if (l.x() <= r.x()) {
        if (l.y() > r.y()) {
            leftTop.setY(r.y());
            rightDown.setY(l.y());
        }
        return;
    }

    if (l.y() < r.y()) {
        leftTop.setX(r.x());
        rightDown.setX(l.x());
        return;
    }

    QPoint tmp;
    tmp = leftTop;
    leftTop = rightDown;
    rightDown = tmp;
}

QScopedPointer<ScreenWidget> ScreenWidget::self;
ScreenWidget *ScreenWidget::Instance(bool need_auto_copy, bool need_auto_save_as)
{
    // 确保每次初始化都是最新的
    self.reset(new ScreenWidget(nullptr, need_auto_copy, need_auto_save_as));
    return self.data();
}

ScreenWidget::ScreenWidget(QWidget *parent, bool auto_copy, bool auto_save_as) : QWidget(parent)
{
    menu = new QMenu(this);
    menu->addAction("复制当前截图", this, SLOT(saveScreen()));
    menu->addAction("复制全屏截图", this, SLOT(saveFullScreen()));
    menu->addAction("截图另存为", this, SLOT(saveScreenOther()));
    menu->addAction("全屏另存为", this, SLOT(saveFullOther()));
    menu->addAction("退出截图", this, SLOT(hide()));

    // 获取缩放因子
    pixelRatio = QApplication::primaryScreen()->devicePixelRatio();
    //取得屏幕大小
    int width = QGuiApplication::primaryScreen()->geometry().width();
    int height = QGuiApplication::primaryScreen()->geometry().height();
    screen = new Screen(QSize(width, height));
    //保存全屏图像
    fullScreen = new QPixmap();

    need_auto_copy = auto_copy;
    need_auto_save_as = auto_save_as;

    // 创建按钮容器和布局
    buttonContainer = new QWidget(this);
    buttonContainer->setFixedHeight(38); // 按钮 30，上下 4 的 padding
    buttonContainer->setStyleSheet("background-color: rgba(200, 200, 200, 180);");

    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(5, 5, 5, 5);  // 设置内边距

    // 添加复制按钮
    QPushButton *copyButton = new QPushButton("复制", this);
    connect(copyButton, &QPushButton::clicked, this, &ScreenWidget::saveScreen);
    buttonLayout->addWidget(copyButton);

    // 添加保存按钮
    QPushButton *saveButton = new QPushButton("保存", this);
    connect(saveButton, &QPushButton::clicked, this, &ScreenWidget::saveScreenOther);
    buttonLayout->addWidget(saveButton);

    // 添加退出按钮
    QPushButton *exitButton = new QPushButton("退出", this);
    connect(exitButton, &QPushButton::clicked, this, &ScreenWidget::hide);
    buttonLayout->addWidget(exitButton);

    buttonContainer->setLayout(buttonLayout);
    buttonContainer->hide();  // 初始隐藏按钮栏，等截图区域生成后显示
}

void ScreenWidget::paintEvent(QPaintEvent *)
{
    int x = screen->getLeftUp().x();
    int y = screen->getLeftUp().y();
    int w = screen->getRightDown().x() - x;
    int h = screen->getRightDown().y() - y;

    QPainter painter(this);

    // 设置绿色边框样式
    QPen pen;
    pen.setColor(Qt::green);
    pen.setWidth(2); // 设置边框宽度
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);

    // 绘制半透明背景
    painter.drawPixmap(0, 0, *bgScreen);

    if (w > 0 && h > 0) {
        // 根据缩放因子调整截图区域的实际坐标和尺寸
        QRect sourceRect(x * pixelRatio, y * pixelRatio, w * pixelRatio, h * pixelRatio);
        QRect targetRect(x, y, w, h);

        // 绘制截图区域内容
        painter.drawPixmap(targetRect, *fullScreen, sourceRect);
    }

    // 绘制绿色边框
    painter.drawRect(x, y, w, h);

    // 绘制尺寸信息文本
    pen.setColor(Qt::black);
    painter.setPen(pen);
    painter.drawText(x + w + 2, y + 12,
        tr("%5 x %6")
        .arg(w * pixelRatio)
        .arg(h * pixelRatio));
}


void ScreenWidget::showEvent(QShowEvent *)
{
    QPoint point(-1, -1);
    screen->setStart(point);
    screen->setEnd(point);

    QScreen *pscreen = QApplication::primaryScreen();
    // 默认当前窗口
    *fullScreen = pscreen->grabWindow(0, 0, 0, screen->width(), screen->height());
    printf("width: %d, height: %d", fullScreen->width(), fullScreen->height());

    //设置透明度实现模糊背景
    QPixmap pix(screen->width(), screen->height());
    bgColor = QColor(160, 160, 160, 200);
    pix.fill((bgColor));
    bgScreen = new QPixmap(*fullScreen);
    QPainter p(bgScreen);
    p.drawPixmap(0, 0, pix);
}

void ScreenWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        hide();
    }
}

void ScreenWidget::saveScreen()
{
    int x = screen->getLeftUp().x();
    int y = screen->getLeftUp().y();
    int w = screen->getRightDown().x() - x;
    int h = screen->getRightDown().y() - y;

    // 使用缩放因子还原截图区域的实际分辨率
    QRect captureRect(x * pixelRatio, y * pixelRatio, w * pixelRatio, h * pixelRatio);

    // 使用 captureRect 进行截图保存，确保不因 DPI 放大
    saveScreenToClip(fullScreen->copy(captureRect));

    hide();
}

void ScreenWidget::saveFullScreen()
{
    saveScreenToClip(*fullScreen);
    hide();
}

void ScreenWidget::saveScreenOther()
{
    QString name = QString("%1.png").arg(STRDATETIME);
    QString fileName = QFileDialog::getSaveFileName(this, "保存图片", name, "png Files (*.png)");
    if (!fileName.endsWith(".png")) {
        fileName += ".png";
    }

    if (fileName.length() > 0) {
        int x = screen->getLeftUp().x();
        int y = screen->getLeftUp().y();
        int w = screen->getRightDown().x() - x;
        int h = screen->getRightDown().y() - y;
        fullScreen->copy(x * pixelRatio, y * pixelRatio, w * pixelRatio, h * pixelRatio).save(fileName, "png");
        hide();
    }
}

void ScreenWidget::saveFullOther()
{
    QString name = QString("%1.png").arg(STRDATETIME);
    QString fileName = QFileDialog::getSaveFileName(this, "保存图片", name, "png Files (*.png)");
    if (!fileName.endsWith(".png")) {
        fileName += ".png";
    }

    if (fileName.length() > 0) {
        fullScreen->save(fileName, "png");
        hide();
    }
}

void ScreenWidget::saveScreenToClip(QPixmap screen)
{
    QClipboard *clip = QApplication::clipboard();
    clip->setPixmap(screen);
}

void ScreenWidget::paintOperatorMenu()
{
    int x = screen->getLeftUp().x();
    int y = screen->getLeftUp().y();
    int w = screen->getRightDown().x() - x;
    int h = screen->getRightDown().y() - y;

    // 绘制菜单项
    int buttonX = x;  // 与截图区域左对齐
    int buttonY = y + h + 5;

    // 如果按钮栏会超出窗口底部，进行适当的调整
    if (buttonY + buttonContainer->height() > this->height()) {
        buttonY = this->height() - buttonContainer->height() - 5;  // 防止超出窗口底部
    }
    buttonContainer->move(buttonX, buttonY);
    buttonContainer->resize(w, buttonContainer->height());
    buttonContainer->show();
}

// 鼠标点击事件
void ScreenWidget::mousePressEvent(QMouseEvent *e)
{
    int status = screen->getStatus();

    // 第一次点击鼠标，设置开始位置
    if (status == Screen::CLICK) {
        screen->setStart(e->pos());
    } else if (status == Screen::MOV) {
        // 此时为 move 状态，鼠标点击事件已经释放
        if (screen->isInArea(e->pos()) == false) {
            // 光标不在截图区域里面，重新截图
            screen->setStart(e->pos());
            screen->setStatus(Screen::CLICK);
        } else {
            // 光标在截图区域里面，需要移动截图区域
            movPos = e->pos();
            this->setCursor(Qt::SizeAllCursor);
        }
    }

    buttonContainer->hide();

    this->update();
}

// 鼠标移动事件
void ScreenWidget::mouseMoveEvent(QMouseEvent *e)
{
    int status = screen->getStatus();

    if (status == Screen::CLICK) {
        // 鼠标处于点击状态则持续更新鼠标落点
        screen->setEnd(e->pos());
    } else if (screen->getStatus() == Screen::MOV) {
        QPoint p(e->x() - movPos.x(), e->y() - movPos.y());
        screen->move(p);
        movPos = e->pos();
    }

    buttonContainer->hide();

    this->update();
}

// 鼠标释放事件
void ScreenWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (need_auto_copy) {
        // 调用复制函数
        saveScreen();
        return;
    }

    if (need_auto_save_as) {
        saveScreenOther();
        return;
    }

    if (screen->getStatus() == Screen::CLICK) {
        // 鼠标释放了，结束移动状态
        screen->setStatus(Screen::MOV);
    } else if (screen->getStatus() == Screen::MOV) {
        // 截图区域移动结束
        this->setCursor(Qt::ArrowCursor);
    }

    this->update();

    paintOperatorMenu();
}

void ScreenWidget::contextMenuEvent(QContextMenuEvent *)
{
    this->setCursor(Qt::ArrowCursor);
    menu->exec(cursor().pos());
}
