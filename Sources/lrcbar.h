#ifndef LRCBAR_H
#define LRCBAR_H

#include <QWidget>
#include "lyrics.h"
#include "player.h"
#include <QtWidgets>
#include <QtCore>
#include <QtGui>

namespace Ui {
class LrcBar;
}

class LrcBar : public QWidget
{
    Q_OBJECT

public:
    explicit LrcBar(Lyrics *lrc, Player *plr, QWidget *parent);
    ~LrcBar();

private slots:
    void UpdateTime();
    void settingFont();
    void enableShadow();
    void enableStroke();

private:
    Ui::LrcBar *ui;
    QTimer *timer;
    Lyrics *lyrics;
    Player *player;
    QPoint pos;//用于窗口拖动，存储鼠标坐标
    bool clickOnFrame;
    bool mouseEnter;
    QLinearGradient linearGradient;
    QLinearGradient maskLinearGradient;
    QFont font;//字体
    int shadowMode;//阴影模式'

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);//窗体拖动相关
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void contextMenuEvent(QContextMenuEvent *event);//右键菜单
};

#endif // LRCBAR_H
