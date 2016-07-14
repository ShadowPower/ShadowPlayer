#include "shadowlabel.h"

ShadowLabel::ShadowLabel(QWidget *parent) :
    QLabel(parent)
{
    shadowColor = QColor(255, 255, 255, 128);
    shadowMode = 0;
}

void ShadowLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);//绘图抗锯齿
    painter.setFont(this->font());
    painter.setPen(shadowColor);//取得阴影颜色
    //绘制阴影
    switch (shadowMode) {
    case 0:
        painter.drawText(2, 2, this->width() - 1, this->height() - 1, this->alignment(), this->text());
        break;
    case 1:
        painter.drawText(0, 0, this->width() - 1, this->height() - 1, this->alignment(), this->text());//左上
        painter.drawText(0, 1, this->width() - 1, this->height() - 1, this->alignment(), this->text());//左
        painter.drawText(0, 2, this->width() - 1, this->height() - 1, this->alignment(), this->text());//左下
        painter.drawText(1, 0, this->width() - 1, this->height() - 1, this->alignment(), this->text());//上
        painter.drawText(1, 2, this->width() - 1, this->height() - 1, this->alignment(), this->text());//下
        painter.drawText(2, 0, this->width() - 1, this->height() - 1, this->alignment(), this->text());//右上
        painter.drawText(2, 1, this->width() - 1, this->height() - 1, this->alignment(), this->text());//右
        painter.drawText(2, 2, this->width() - 1, this->height() - 1, this->alignment(), this->text());//右下
        break;
    default:
        painter.drawText(1, 1, this->width(), this->height(), this->alignment(), this->text());
        break;
    }
    painter.setPen(this->palette().color(QPalette::WindowText));//取得文本颜色
    painter.drawText(1, 1, this->width() - 1, this->height() - 1, this->alignment(), this->text());//绘制文本
}

void ShadowLabel::setShadowColor(QColor color)
{
    shadowColor = color;
}

void ShadowLabel::setShadowMode(int mode)
{
    shadowMode = mode;
}
