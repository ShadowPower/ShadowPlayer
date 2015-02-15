#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QWidget>
#include <QtWidgets>
#include <QtCore>
#include <QtGui>
#include "player.h"

namespace Ui {
class PlayList;
}

class PlayList : public QWidget
{
    Q_OBJECT

public:
    explicit PlayList(Player *player, QWidget *parent = 0);
    ~PlayList();
    bool fixSuffix(QString fileName);
    bool isEmpty();
    void add(QString fileName);//添加文件
    void insert(int index, QString fileName);//插入文件
    void remove(int index);
    void clearAll();
    int getLength();
    int getIndex();//取得当前播放索引
    QString next(bool isLoop = false);//得到下一曲路径，索引+1 参数：是否循环
    QString previous(bool isLoop = false);//得到上一曲地址路径
    QString playIndex(int index);//播放指定索引
    QString getFileNameForIndex(int index);
    QString getCurFile();
    QString playLast();//播放列表末尾文件
    void tableUpdate();//更新显示内容
    void saveToFile(QString fileName);
    void readFromFile(QString fileName);

private slots:
    void on_deleteButton_clicked();
    void on_playListTable_cellDoubleClicked(int row, int);
    void on_clearButton_clicked();
    void on_insertButton_clicked();
    void on_addButton_clicked();
    void on_searchButton_clicked();
    void on_searchNextButton_clicked();
    void on_setLenFilButton_clicked();

signals:
    void callPlayer();//请求主窗口播放文件

private:
    Ui::PlayList *ui;
    QList<QString> fileList;
    QList<QString> timeList;
    int curIndex;//正在播放的文件索引
    Player *player;
    int lengthFilter;//播放长度过滤

protected:
    void dragEnterEvent(QDragEnterEvent *event);//拖放相关
    void dropEvent(QDropEvent *event);
};

#endif // PLAYLIST_H
