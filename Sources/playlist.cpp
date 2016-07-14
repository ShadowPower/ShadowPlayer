#include "playlist.h"
#include "ui_playlist.h"

PlayList::PlayList(Player *player, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayList)
{
    ui->setupUi(this);
    curIndex = 0;
    lengthFilter = 0;
    this->player = player;
    ui->playListTable->horizontalHeader()->setStretchLastSection(true);//自动设置最后一列宽度
    this->setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    this->setFixedSize(this->width(), this->height());

    //“查找”文本框，绑定回车键到“查找”按钮
    connect(ui->searchEdit, SIGNAL(returnPressed()), this, SLOT(on_searchButton_clicked()));

    readFromFile(QCoreApplication::applicationDirPath() + "/PlayList.sdpl");
}

PlayList::~PlayList()
{
    saveToFile(QCoreApplication::applicationDirPath() + "/PlayList.sdpl");
    delete ui;
}

bool PlayList::fixSuffix(QString fileName)
{
    if(fileName.toLower().endsWith(".mp3")||
            fileName.toLower().endsWith(".mp2")||
            fileName.toLower().endsWith(".mp1")||
            fileName.toLower().endsWith(".wav")||
            fileName.toLower().endsWith(".ogg")||
            fileName.toLower().endsWith(".aiff")||
            fileName.toLower().endsWith(".ape")||
            fileName.toLower().endsWith(".mp4")||
            fileName.toLower().endsWith(".m4a")||
            fileName.toLower().endsWith(".m4v")||
            fileName.toLower().endsWith(".aac")||
            fileName.toLower().endsWith(".alac")||
            fileName.toLower().endsWith(".tta")||
            fileName.toLower().endsWith(".flac")||
            fileName.toLower().endsWith(".wma")||
            fileName.toLower().endsWith(".wv")
            )
    {
        return true;
    } else {
        return false;
    }
}

bool PlayList::isEmpty()
{
    if (fileList.isEmpty())
        return true;
    else
        return false;
}

//核心函数，添加歌曲
void PlayList::add(QString fileName)
{
    if (fixSuffix(fileName))
    {
        if ((int)(player->getFileSecond(fileName)) >= lengthFilter)
        {
            fileList.append(fileName);
            timeList.append(player->getFileTotalTime(fileName));
        }
    }
    tableUpdate();
}

//核心函数
void PlayList::insert(int index, QString fileName)
{
    if (fixSuffix(fileName))
    {
        if ((int)(player->getFileSecond(fileName)) >= lengthFilter)
        {
            if (index < curIndex)
                ++curIndex;//如果在前端插入，后移播放索引
            fileList.insert(index, fileName);
            timeList.insert(index, player->getFileTotalTime(fileName));
        }
    }
    tableUpdate();
}

void PlayList::remove(int index)
{
    if (index <= curIndex && index > -1)//大于-1用于判断是否未选任何项
        --curIndex;//删除前方项，上移当前项索引
    fileList.removeAt(index);
    timeList.removeAt(index);
    tableUpdate();
}

void PlayList::clearAll()
{
    fileList.clear();
    timeList.clear();
    curIndex = 0;
    tableUpdate();
}

int PlayList::getLength()
{
    return fileList.length();
}

int PlayList::getIndex()
{
    if (!fileList.isEmpty())
    {
        return curIndex;
    }
    else
    {
        return -1;
    }
}

//下一曲 参数：是否循环
QString PlayList::next(bool isLoop)
{
    if (!fileList.isEmpty())
    {
        if (isLoop)
        {
            if (curIndex < fileList.length() - 1)
            {
                ++curIndex;
            } else {
                curIndex = 0;
            }
            ui->playListTable->selectRow(curIndex);//让视图跟随当前播放歌曲，下一句使该行不选中
            tableUpdate();
            return fileList[curIndex];
        } else {
            if (curIndex < fileList.length() - 1)
            {
                ++curIndex;
                ui->playListTable->selectRow(curIndex);
                tableUpdate();
                return fileList[curIndex];
            } else {
                return "stop";//后面没有了。此信息将不处理
            }
        }
    }
    return "";//空列表
}

//上一曲
QString PlayList::previous(bool isLoop)
{
    if (!fileList.isEmpty())
    {
        if (isLoop)
        {
            if (curIndex == 0)
            {
                curIndex = fileList.length() - 1;//循环读取
            } else {
                --curIndex;
            }
            ui->playListTable->selectRow(curIndex);
            tableUpdate();
            return fileList[curIndex];
        } else {
            if (curIndex > 0)
            {
                --curIndex;
                ui->playListTable->selectRow(curIndex);
                tableUpdate();
                return fileList[curIndex];
            } else {
                return "stop";//前面没有了。
            }
        }
    }
    return "";//空列表
}

//index从0开始计
QString PlayList::playIndex(int index)
{
    curIndex = index;
    ui->playListTable->selectRow(curIndex);
    tableUpdate();
    return fileList[curIndex];
}

//返回指定索引的文件名
QString PlayList::getFileNameForIndex(int index)
{
    return fileList[index];
}

//返回当前文件名
QString PlayList::getCurFile()
{
    return fileList[curIndex];
}

//返回最后一个文件名，并设置索引
QString PlayList::playLast()
{
    if (!fileList.isEmpty())
    {
        curIndex = fileList.length() - 1;
        ui->playListTable->selectRow(curIndex);
        tableUpdate();
        return fileList[curIndex];
    } else {
        return "";
    }
}

//刷新表格
void PlayList::tableUpdate()
{
    //这里不能调用此类中任何一个操作元素的函数
    //否则将造成循环递归
    ui->playListTable->clear();
    ui->playListTable->setRowCount(getLength());//删除项目后，更改表格总行数，防止出现空白行。
    int count = fileList.size();//循环效率优化
    //重新生成表格项目
    for (int i = 0; i < count; i++)
    {
        QString fileName = fileList[i];
        QFileInfo fileInfo(fileName);

        QTableWidgetItem *item = new QTableWidgetItem(fileInfo.fileName());
        QTableWidgetItem *timeItem = new QTableWidgetItem(timeList[i]);

        if (i == curIndex)
        {
            item->setBackgroundColor(QColor(128, 255, 0, 128));
            timeItem->setBackgroundColor(QColor(128, 255, 0, 128));
        }

        ui->playListTable->setItem(i, 0, item);
        ui->playListTable->setItem(i, 1, timeItem);
    }
}

void PlayList::on_deleteButton_clicked()
{
    /*
     * 这里做了好久，若直接删除对应行号，上次删除操作会使行号发生变动，误删
     * 试过遍历列表得到选中的项目，倒序访问，结果发现排列方式可能不是倒序（访问越界）
     * 试过顺序+偏移量的方式，后几项误删
     * 以下方法仍为倒序删除。
     * 测试正常
     * 此方法来自：http://hi.baidu.com/speedylvshirly/item/27c37571f56deb19d1dcb3ee
     */

    //获得列表模型
    QItemSelectionModel *selectionModel = ui->playListTable->selectionModel();
    //获得选择项的列表
    QModelIndexList selected = selectionModel->selectedIndexes();
    QMap<int, int> rowMap;//利用QMap的自动排序功能来排列顺序以便删除
    foreach (QModelIndex index, selected)
    {
        rowMap.insert(index.row(), 0);//遍历元素，取得要删除的行数Map
    }

    int row;//要删除的行
    //创建一个迭代器
    QMapIterator<int, int> rowMapIterator(rowMap);
    rowMapIterator.toBack();
    //循环倒序删除
    while (rowMapIterator.hasPrevious())
    {
        rowMapIterator.previous();
        row = rowMapIterator.key();
        remove(row);
    }
}

void PlayList::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void PlayList::dropEvent(QDropEvent *event)
{
    /*
     * 不同于主窗口
     * 允许多文件拖放
     */
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    int urlCount = urls.size();
    QString fileName;

    for (int i = 0; i < urlCount; i++)
    {
        fileName = urls[i].toLocalFile();
        if (fixSuffix(fileName))
        {
            add(fileName);
        }
    }
}

void PlayList::on_playListTable_cellDoubleClicked(int row, int )
{
    curIndex = row;
    emit callPlayer();//发送信号给播放器
    tableUpdate();
}

void PlayList::on_clearButton_clicked()
{
    clearAll();
}

void PlayList::on_insertButton_clicked()
{
    int index = ui->playListTable->currentRow();//记录当前选中的列
    if (index < 0)
        index = 0;//如果没有选中任何项，在开头插入
    QStringList fileNames = QFileDialog::getOpenFileNames(this, QString::fromUtf8("在选择项前插入"), 0, QString::fromUtf8("音频文件 (*.mp3 *.mp2 *.mp1 *.wav *.aiff *.ogg *.ape *.mp4 *.m4a *.m4v *.aac *.alac *.tta *.flac *.wma *.wv)"));
    int count = fileNames.size();
    for (int i = 0; i < count; i++)
    {
        QString fileName = fileNames[i];
        insert(index + i, fileName);
    }
}

void PlayList::on_addButton_clicked()
{
    //QFileDialog存在莫名其妙的内存泄漏问题
    //选中文件会占用内存空间
    //取消选中不会释放
    //无解……
    QStringList fileNames = QFileDialog::getOpenFileNames(this, QString::fromUtf8("添加音频"), 0, QString::fromUtf8("音频文件 (*.mp3 *.mp2 *.mp1 *.wav *.aiff *.ogg *.ape *.mp4 *.m4a *.m4v *.aac *.alac *.tta *.flac *.wma *.wv)"));
    int count = fileNames.size();
    for (int i = 0; i < count; i++)
    {
        QString fileName = fileNames[i];
        add(fileName);
    }
}

//查找项目
void PlayList::on_searchButton_clicked()
{
    if (!fileList.isEmpty() && !ui->searchEdit->text().isEmpty())
    {
        int resultIndex = -1;
        int count = fileList.size();
        //在列表中查找字符串
        for (int i = 0; i < count; i++)
        {
            QString fileName = fileList[i];
            QFileInfo fileInfo(fileName);

            //判断“区分大小写”复选框
            if (ui->isCaseSensitive->isChecked())
            {
                if (fileInfo.fileName().indexOf(ui->searchEdit->text()) > -1)//区分大小写
                {
                    //得到结果，记录
                    resultIndex = i;
                    break;
                }
            } else {
                if (fileInfo.fileName().toLower().indexOf(ui->searchEdit->text().toLower()) > -1)//不区分大小写
                {
                    resultIndex = i;
                    break;
                }
            }
        }

        //如果查找到结果
        if (resultIndex != -1)
        {
            ui->playListTable->selectRow(resultIndex);
        } else {
            QMessageBox::information(0, "很抱歉", "找不到您要查找的内容", "谢谢");
        }
    } else if (ui->searchEdit->text().isEmpty()) {
        QMessageBox::information(0, "您好", "请问您要查找什么？", "我忘记了");
    } else {
        QMessageBox::information(0, "这是什么情况", "明明什么都没有，为什么我还要在里面找东西呢？。。", "~(>_<。)＼");
    }
}

//查找下一个
void PlayList::on_searchNextButton_clicked()
{
    if (!fileList.isEmpty() && !ui->searchEdit->text().isEmpty())
    {
        int resultIndex = -1;
        int start = ui->playListTable->currentRow() + 1;
        int count = fileList.size();

        if (start < count) //检测是否越界
            for (int i = start; i < count; i++)
            {
                QString fileName = fileList[i];
                QFileInfo fileInfo(fileName);

                if (ui->isCaseSensitive->isChecked())
                {
                    if (fileInfo.fileName().indexOf(ui->searchEdit->text()) > -1)
                    {

                        resultIndex = i;
                        break;
                    }
                } else {
                    if (fileInfo.fileName().toLower().indexOf(ui->searchEdit->text().toLower()) > -1)
                    {
                        resultIndex = i;
                        break;
                    }
                }
            }

        if (resultIndex != -1)
        {
            ui->playListTable->selectRow(resultIndex);
        } else {
            QMessageBox::information(0, "已完成查找", "所有的内容都查找完毕啦", "辛苦了");
        }
    } else if (ui->searchEdit->text().isEmpty()) {
        QMessageBox::information(0, "您好", "请问您要查找什么呢？", "我忘记了");
    } else {
        QMessageBox::information(0, "这是什么情况", "明明什么都没有，为什么我还要在里面找东西呢？。。", "~(>_<。)＼");
    }
}

void PlayList::on_setLenFilButton_clicked()
{
    bool ok;
    int set = QInputDialog::getInt(0, "最小播放长度", "小于该长度的音频文件将不被接受\n单位：秒" ,lengthFilter, 0, 2147483647, 1, &ok);
    if (ok)
        lengthFilter = set;
}

//保存播放列表，参数：文件路径
void PlayList::saveToFile(QString fileName)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream << (quint32)0x61727487 << fileList << timeList << curIndex;
    file.close();
}

//载入播放列表，参数：文件路径
void PlayList::readFromFile(QString fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    quint32 magic;
    stream >> magic;
    if (magic == 0x61727487)
    {
        stream >> fileList;
        stream >> timeList;
        stream >> curIndex;
    }
    file.close();
    tableUpdate();
}
