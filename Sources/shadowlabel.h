#ifndef SHADOWLABEL_H
#define SHADOWLABEL_H

#include <QLabel>
#include <QtWidgets>
#include <QtGui>
#include <QtCore>

class ShadowLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ShadowLabel(QWidget *parent = 0);
    void setShadowColor(QColor color);
    void setShadowMode(int mode);

signals:

public slots:

private:
    QColor shadowColor;
    int shadowMode;

protected:
    void paintEvent(QPaintEvent *event);

};

#endif // SHADOWLABEL_H
