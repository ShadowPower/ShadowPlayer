/*
 * 高低16位定义：
 * #define LOWORD(a) (WORD)(a)
 * #define HIWORD(a) (WORD)((a)>>16)
 */
#include "shadowplayer.h"
#include "ui_shadowplayer.h"
#include "ID3v2Pic.h"
#include "FlacPic.h"

/*
 * QFileDialog中存在内存泄漏BUG
 * 此BUG在QtCreator中同样存在……
 *
 * 倒放状态下，点击停止或播放结束，定位在末尾。
 * 倒放状态下，使用单曲播放模式时，存在一个BUG：不能在播放结束后定位到末尾
 *
 * 在Windows系统下，均衡器的频率范围只有60~16000
 * 在Linux/Mac/Android/iOS系统下（如果有移植意向的话……），均衡器的频率范围 0Hz~(音频采样率的一半)Hz
 * （此程序曾移植到Linux下，无法正常使用……）
 * （错误：文件格式不支持……）
 */

ShadowPlayer::ShadowPlayer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ShadowPlayer)
{
    ui->setupUi(this);
    //this->setFixedSize(this->width(), this->height());//锁定窗口大小

    player = new Player();//播放功能封装
    lyrics = new Lyrics();//歌词功能封装
    playList = new PlayList(player, ui->playerListArea);
    osd = new OSD();
    timer = new QTimer(this);//定时器
    lrcTimer = new QTimer(this);//歌词显示定时器

    lb = new LrcBar(lyrics, player, 0);//传递对象指针以便访问
    playing = false;
    this->setWindowIcon(QIcon(":icon/ICO/ShadowPlayer.ico"));//设置窗口图标
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);//无边框
    this->setAttribute(Qt::WA_TranslucentBackground, true);//背景透明
    ui->coverLabel->setScaledContents(true);//自动缩放内容
    ui->coverLabel->setPixmap(QPixmap(":image/image/ShadowPlayer.png"));

    //阴影标签相关设置
    ui->tagLabel->setShadowMode(1);
    ui->mediaInfoLabel->setShadowMode(1);
    ui->tagLabel->setShadowColor(QColor(0, 0, 0, 80));
    ui->mediaInfoLabel->setShadowColor(QColor(0, 0, 0, 80));
    ui->curTimeLabel->setShadowMode(1);
    ui->totalTimeLabel->setShadowMode(1);
    ui->curTimeLabel->setShadowColor(QColor(0, 0, 0, 80));
    ui->totalTimeLabel->setShadowColor(QColor(0, 0, 0, 80));

    player->devInit();

    //信号与槽的关联
    connect(timer,SIGNAL(timeout()),this,SLOT(UpdateTime()));
    connect(lrcTimer,SIGNAL(timeout()),this,SLOT(UpdateLrc()));
    //均衡器
    connect(ui->eqSlider_1, SIGNAL(valueChanged(int)), this, SLOT(applyEQ()));
    connect(ui->eqSlider_2, SIGNAL(valueChanged(int)), this, SLOT(applyEQ()));
    connect(ui->eqSlider_3, SIGNAL(valueChanged(int)), this, SLOT(applyEQ()));
    connect(ui->eqSlider_4, SIGNAL(valueChanged(int)), this, SLOT(applyEQ()));
    connect(ui->eqSlider_5, SIGNAL(valueChanged(int)), this, SLOT(applyEQ()));
    connect(ui->eqSlider_6, SIGNAL(valueChanged(int)), this, SLOT(applyEQ()));
    connect(ui->eqSlider_7, SIGNAL(valueChanged(int)), this, SLOT(applyEQ()));
    connect(ui->eqSlider_8, SIGNAL(valueChanged(int)), this, SLOT(applyEQ()));
    connect(ui->eqSlider_9, SIGNAL(valueChanged(int)), this, SLOT(applyEQ()));
    connect(ui->eqSlider_10, SIGNAL(valueChanged(int)), this, SLOT(applyEQ()));

    connect(playList, SIGNAL(callPlayer()), this, SLOT(callFromPlayList()));

    MyGlobalShortCut *playShortcut = new MyGlobalShortCut("Ctrl+F2", this);
    connect(playShortcut,SIGNAL(activated()), this,SLOT(on_playButton_clicked()));//播放键热键
    MyGlobalShortCut *stopShortcut = new MyGlobalShortCut("Ctrl+F3", this);
    connect(stopShortcut,SIGNAL(activated()), this,SLOT(on_stopButton_clicked()));//停止键热键
    MyGlobalShortCut *muteShortcut = new MyGlobalShortCut("Ctrl+F4", this);
    connect(muteShortcut,SIGNAL(activated()), this,SLOT(on_muteButton_clicked()));//下一曲热键

    MyGlobalShortCut *preShortcut = new MyGlobalShortCut("Ctrl+F5", this);
    connect(preShortcut,SIGNAL(activated()), this,SLOT(on_playPreButton_clicked()));//上一曲热键
    MyGlobalShortCut *nextShortcut = new MyGlobalShortCut("Ctrl+F6", this);
    connect(nextShortcut,SIGNAL(activated()), this,SLOT(on_playNextButton_clicked()));//下一曲热键

    skinMode = 2; //皮肤模式 0.不更改大小 1.左侧 2.全窗口 3.自动 4.动态缩放
    skinPos = 1;//背景图片位置 0.显示顶端 1.显示中间 2.显示底部
    skinDrawPos = 0;//绘制背景图片时，使用的百分比位置 0 = 顶部 1 = 底部
    aspectRatio = 0;//图片宽高比，用于动态缩放

    playMode = 2;//播放模式 0.单曲播放 1.单曲循环 2.顺序播放 3.列表循环 4.随机播放
    isReverse = false;//默认不倒放

    clickOnFrame = false;//是否点击到窗体（面板）上，用于解决点击控件后窗口瞬移的BUG
    clickOnLeft = false;

    isMute = false;//是否静音
    isPlaySliderPress = false;

    timer->start(27);//定时器太慢会触发进度条setValue的BUG
    lrcTimer->start(70);

    setAcceptDrops(true);//允许拖放
    loadSkin(":/image/image/Skin1.jpg", false);//应用默认皮肤
    ui->playerListArea->setGeometry(QRect(370, 200, 331, 0));//将播放列表放在默认位置

    //载入配置文件
    loadConfig();

    //背景渐变
    bgLinearGradient.setColorAt(0, QColor(255, 255, 255, 0));
    bgLinearGradient.setColorAt(1, QColor(255, 255, 255, 255));
    bgLinearGradient.setStart(0, 0);
    bgLinearGradient.setFinalStop(0, height());

    //Nt 6.x相关
    //任务栏进度条初始化
    taskbarButton = new QWinTaskbarButton(this);
    taskbarButton->setWindow(windowHandle());
    taskbarProgress = taskbarButton->progress();
    taskbarProgress->setRange(0, 1000);
    connect(ui->playSlider, SIGNAL(valueChanged(int)), taskbarProgress, SLOT(setValue(int)));

    //任务栏缩略图按钮
    thumbnailToolBar = new QWinThumbnailToolBar(this);
    playToolButton = new QWinThumbnailToolButton(thumbnailToolBar);
    stopToolButton = new QWinThumbnailToolButton(thumbnailToolBar);
    backwardToolButton = new QWinThumbnailToolButton(thumbnailToolBar);
    forwardToolButton = new QWinThumbnailToolButton(thumbnailToolBar);
    playToolButton->setToolTip("播放");
    playToolButton->setIcon(QIcon(":/icon/ICO/Play.png"));
    stopToolButton->setToolTip("停止");
    stopToolButton->setIcon(QIcon(":/icon/ICO/Stop.png"));
    backwardToolButton->setToolTip("上一曲");
    backwardToolButton->setIcon(QIcon(":/icon/ICO/Pre.png"));
    forwardToolButton->setToolTip("下一曲");
    forwardToolButton->setIcon(QIcon(":/icon/ICO/Next.png"));
    thumbnailToolBar->addButton(playToolButton);
    thumbnailToolBar->addButton(stopToolButton);
    thumbnailToolBar->addButton(backwardToolButton);
    thumbnailToolBar->addButton(forwardToolButton);
    connect(playToolButton, SIGNAL(clicked()), this, SLOT(on_playButton_clicked()));
    connect(stopToolButton, SIGNAL(clicked()), this, SLOT(on_stopButton_clicked()));
    connect(backwardToolButton, SIGNAL(clicked()), this, SLOT(on_playPreButton_clicked()));
    connect(forwardToolButton, SIGNAL(clicked()), this, SLOT(on_playNextButton_clicked()));

    //初始化动画

    //窗口展开收缩动画
    sizeSlideAnimation = new QPropertyAnimation(this, "geometry");
    sizeSlideAnimation->setDuration(700);
    sizeSlideAnimation->setEasingCurve(QEasingCurve::OutCirc);

    //载入文件时，三个标签的动画
    tagAnimation = new QPropertyAnimation(ui->tagLabel, "geometry");
    tagAnimation->setDuration(700);
    tagAnimation->setStartValue(QRect(130, 30, 0, 16));
    tagAnimation->setEndValue(QRect(130, 30, 221, 16));
    mediaInfoAnimation = new QPropertyAnimation(ui->mediaInfoLabel, "geometry");
    mediaInfoAnimation->setDuration(800);
    mediaInfoAnimation->setStartValue(QRect(130, 50, 0, 16));
    mediaInfoAnimation->setEndValue(QRect(130, 50, 221, 16));
    coverAnimation = new QPropertyAnimation(ui->coverLabel, "geometry");
    coverAnimation->setEasingCurve(QEasingCurve::OutCirc);
    coverAnimation->setDuration(600);
    coverAnimation->setStartValue(QRect(60, 60, 1, 1));
    coverAnimation->setEndValue(QRect(10, 10, 111, 111));

    //播放列表相关
    //隐藏均衡器
    eqHideAnimation = new QPropertyAnimation(ui->eqGroupBox, "geometry");
    eqHideAnimation->setDuration(600);
    eqHideAnimation->setStartValue(QRect(370, 30, 331, 171));
    eqHideAnimation->setEndValue(QRect(370, 30, 331, 0));

    //显示均衡器
    eqShowAnimation = new QPropertyAnimation(ui->eqGroupBox, "geometry");
    eqShowAnimation->setDuration(600);
    eqShowAnimation->setStartValue(QRect(370, 30, 331, 0));
    eqShowAnimation->setEndValue(QRect(370, 30, 331, 171));

    //隐藏歌词
    lyricsHideAnimation = new QPropertyAnimation(ui->lyricsBox, "geometry");
    lyricsHideAnimation->setDuration(600);
    lyricsHideAnimation->setStartValue(QRect(370, 210, 331, 181));
    lyricsHideAnimation->setEndValue(QRect(370, 391, 331, 0));

    //显示歌词
    lyricsShowAnimation = new QPropertyAnimation(ui->lyricsBox, "geometry");
    lyricsShowAnimation->setDuration(600);
    lyricsShowAnimation->setStartValue(QRect(370, 391, 331, 0));
    lyricsShowAnimation->setEndValue(QRect(370, 210, 331, 181));

    //隐藏播放列表
    playListHideAnimation = new QPropertyAnimation(ui->playerListArea, "geometry");
    playListHideAnimation->setDuration(600);
    playListHideAnimation->setStartValue(QRect(370, 30, 331, 361));
    playListHideAnimation->setEndValue(QRect(370, 205, 331, 0));

    //显示播放列表
    playListShowAnimation = new QPropertyAnimation(ui->playerListArea, "geometry");
    playListShowAnimation->setDuration(600);
    playListShowAnimation->setStartValue(QRect(370, 205, 331, 0));
    playListShowAnimation->setEndValue(QRect(370, 30, 331, 361));

    connect(playListHideAnimation, SIGNAL(finished()), this, SLOT(update()));
    connect(playListShowAnimation, SIGNAL(finished()), this, SLOT(update()));//动画完成后刷新窗口

    //淡出动画，动画结束后程序退出
    fadeOutAnimation = new QPropertyAnimation(this, "windowOpacity");
    fadeOutAnimation->setDuration(400);
    fadeOutAnimation->setStartValue(1);
    fadeOutAnimation->setEndValue(0);
    connect(fadeOutAnimation, SIGNAL(finished()), this, SLOT(close()));

    //淡入窗口
    fadeInAnimation = new QPropertyAnimation(this, "windowOpacity");
    fadeInAnimation->setDuration(400);
    fadeInAnimation->setStartValue(0);
    fadeInAnimation->setEndValue(1);
    fadeInAnimation->start();
    connect(fadeInAnimation, SIGNAL(finished()), this, SLOT(setTaskbarButtonWindow()));//当窗口淡入完毕后自动设置任务栏按钮
}

ShadowPlayer::~ShadowPlayer()
{
    saveConfig();
    delete ui;
    delete lb;
    delete player;
    delete timer;
    delete lyrics;
    delete playList;
    delete osd;
    delete sizeSlideAnimation;
    delete fadeInAnimation;
    delete tagAnimation;
    delete mediaInfoAnimation;
    delete coverAnimation;
    delete fadeOutAnimation;
    delete eqHideAnimation;
    delete eqShowAnimation;
    delete lyricsHideAnimation;
    delete lyricsShowAnimation;
    delete playListHideAnimation;
    delete playListShowAnimation;
}

void ShadowPlayer::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void ShadowPlayer::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty())
        return;

    if(fileName.toLower().endsWith(".jpg")||
            fileName.toLower().endsWith(".jpeg")||
            fileName.toLower().endsWith(".png")||
            fileName.toLower().endsWith(".gif")||
            fileName.toLower().endsWith(".bmp"))
    {
        loadSkin(fileName);
    } else if (fileName.toLower().endsWith(".lrc")) {
        lyrics->resolve(fileName, true);
    } else {
        addToListAndPlay(urls);
    }
}

void ShadowPlayer::paintEvent(QPaintEvent *)
{
    //覆盖绘图函数绘制背景图片
    QPainter painter(this);

    //如果存在背景图片
    if (!skin.isNull())
    {

        int topY = 0;//图片顶部坐标

        switch (skinPos) {
        case 0:
            //显示顶部
            skinDrawPos = 0;
            break;
        case 1:
            //图片居中
            skinDrawPos = 0.5;
            break;
        case 2:
            //显示底部
            skinDrawPos = 1;
            break;
        default:
            break;
        }

        //绘制图片
        switch (skinMode) {
        case 0:
            //原始大小
            topY = -(skin.height() - 400) * skinDrawPos;
            painter.drawPixmap(0, topY, skin);
            break;
        case 1:
            //左侧宽度
            topY = -(skinLeft.height() - 400) * skinDrawPos;
            painter.drawPixmap(0, topY, skinLeft);
            break;
        case 2:
            //全窗口宽度
            topY = -(skinFull.height() - 400) * skinDrawPos;
            painter.drawPixmap(0, topY, skinFull);
            break;
        case 3:
            //自动缩放
            if (this->geometry().width() > 361)
            {
                topY = -(skinFull.height() - 400) * skinDrawPos;
                painter.drawPixmap(0, topY, skinFull);
            } else {
                topY = -(skinLeft.height() - 400) * skinDrawPos;
                painter.drawPixmap(0, topY, skinLeft);
            }
            break;
        case 4:
            //动态缩放
            if (this->geometry().width() <= 360) //若窗口为最小，绘制高质量小图
            {
                topY = -(skinLeft.height() - 400) * skinDrawPos;
                painter.drawPixmap(0, topY, skinLeft);
            } else if (this->geometry().width() >= 710) { //若窗口为最大，绘制高质量大图
                topY = -(skinFull.height() - 400) * skinDrawPos;
                painter.drawPixmap(0, topY, skinFull);
            } else { //其余情况
                topY = -(this->width()*aspectRatio - 400) * skinDrawPos; //根据宽高比比来计算位置
                painter.drawPixmap(0, topY, this->width(), this->width()*aspectRatio, skin); //保持宽高比绘制
            }
            break;
        default:
            break;
        }
    }

    //绘制背景渐变
    painter.setBrush(QBrush(bgLinearGradient));
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, width(), height());
}

//右键菜单
void ShadowPlayer::contextMenuEvent(QContextMenuEvent *)
{
    QCursor cur=this->cursor();

    QAction textAction1("拖动窗口左侧可以精确调节位置", this);
    textAction1.setEnabled(false);

    QMenu menu;
    menu.addAction("载入默认皮肤" , this, SLOT(loadDefaultSkin()));
    menu.addSeparator();
    menu.addAction("适配皮肤到左窗口（适合较窄图片）", this, SLOT(fixSkinSizeLeft()));
    menu.addAction("适配皮肤到全窗口（适合较宽图片）", this, SLOT(fixSkinSizeFull()));
    menu.addAction("不更改皮肤大小", this, SLOT(originalSkinSize()));
    menu.addAction("根据窗口大小自动调整皮肤", this, SLOT(autoSkinSize()));
    menu.addAction("动态缩放皮肤", this, SLOT(dynamicSkinSize()));
    menu.addSeparator();
    menu.addAction("背景图顶部显示", this, SLOT(skinOnTop()));
    menu.addAction("背景图居中显示", this, SLOT(skinOnCenter()));
    menu.addAction("背景图底部显示", this, SLOT(skinOnButtom()));
    menu.addAction(&textAction1);
    menu.addSeparator();
    menu.addAction("不使用皮肤（优化性能）", this, SLOT(skinDisable()));
    menu.addSeparator();
    menu.addAction("设置物理参数（毫无人性）", this, SLOT(physicsSetting()));
    menu.addAction("载入预设 - 物理版频谱", this, SLOT(enableFFTPhysics()));
    menu.addAction("载入预设 - 正常版频谱", this, SLOT(disableFFTPhysics()));
    menu.addSeparator();
    menu.addAction("关于作者", this, SLOT(showDeveloperInfo()));
    menu.exec(cur.pos());
}

//载入默认背景
void ShadowPlayer::loadDefaultSkin()
{
    loadSkin(":/image/image/Skin1.jpg", false);
    QFile skinFile(QCoreApplication::applicationDirPath() + "/skin.dat");
    skinFile.remove();
}

//更改皮肤模式的实现
void ShadowPlayer::fixSkinSizeLeft()
{
    skinMode = 1;
    this->repaint();//使用update函数无效果，原因不明，改用repaint
}

void ShadowPlayer::fixSkinSizeFull()
{
    skinMode = 2;
    this->repaint();
}

void ShadowPlayer::originalSkinSize()
{
    skinMode = 0;
    this->repaint();
}

void ShadowPlayer::autoSkinSize()
{
    skinMode = 3;
    this->repaint();
}

void ShadowPlayer::dynamicSkinSize()
{
    skinMode = 4;
    this->repaint();
}

void ShadowPlayer::skinOnTop()
{
    skinPos = 0;
    this->repaint();
}

void ShadowPlayer::skinOnCenter()
{
    skinPos = 1;
    this->repaint();
}

void ShadowPlayer::skinOnButtom()
{
    skinPos = 2;
    this->repaint();
}

//禁用皮肤
void ShadowPlayer::skinDisable()
{
    skin = QPixmap();
    skinLeft = skin;
    skinFull = skin;
    this->repaint();
}

//播放界面三个标签的动画效果
void ShadowPlayer::infoLabelAnimation()
{
    tagAnimation->stop();
    mediaInfoAnimation->stop();
    coverAnimation->stop();

    tagAnimation->start();
    mediaInfoAnimation->start();
    coverAnimation->start();
    this->update();//刷新窗口
}

void ShadowPlayer::loadFile(QString file)
{
    //载入文件
    if (player->openFile(file) != "err")
    {
        QFileInfo fileinfo(file);
        showCoverPic(file);//显示专辑封面
        //this->setWindowTitle("ShadowPlayer - " + fileinfo.fileName());
        ui->mediaInfoLabel->setText(player->getNowPlayInfo());
        ui->totalTimeLabel->setText(player->getTotalTime());
        oriFreq = player->getFreq();//存储初始采样率
        //ui->freqSlider->setSliderPosition(0);//恢复采样率设置条默认设置
        on_freqSlider_valueChanged(ui->freqSlider->value());//设置采样率
        player->setVol(ui->volSlider->value());//设置音量
        applyEQ();//设置均衡器

        //设置倒放
        if (isReverse)
            player->setReverse(true);
        else
            player->setReverse(false);

        //设置均衡器
        on_eqEnableCheckBox_clicked(ui->eqEnableCheckBox->isChecked());

        //更新混响参数
        player->updateReverb(ui->reverbDial->value());

        //----------设置播放按钮状态&启动播放
        player->play();//启动播放
        if (player->isPlaying())//检测播放状态
        {
            playing = true;
            ui->playButton->setIcon(QIcon(":/icon/ICO/Pause.png"));
            ui->playButton->setToolTip("暂停（Ctrl+F2）");
            taskbarProgress->show();
            taskbarProgress->resume();
            taskbarButton->setOverlayIcon(QIcon(":/icon/ICO/Play.png"));
            playToolButton->setIcon(QIcon(":/icon/ICO/Pause.png"));
            playToolButton->setToolTip("暂停");
        }
        //----------结束设置播放按钮&启动播放

        /*
        playing = false;//设置状态以便下一步播放
        on_playButton_clicked();//点击播放按钮，开始播放
        */

        if (!lyrics->resolve(file))//载入文件目录下的歌词文件
            if(!lyrics->loadFromLrcDir(file))//失败则载入程序目录下的歌词
                if(!lyrics->loadFromFileRelativePath(file, "/Lyrics/"))//失败则载入文件目录下子歌词目录的歌词
                    lyrics->loadFromFileRelativePath(file, "/../Lyrics/");//新版百度音乐的路径

        if (player->getTags() == "Show_File_Name")//Show_File_Name = 要显示文件名
            ui->tagLabel->setText(fileinfo.fileName());
        else
            ui->tagLabel->setText(player->getTags());

        infoLabelAnimation();//使用动画效果
        osd->showOSD(ui->tagLabel->text(), player->getTotalTime());
    }
}

void ShadowPlayer::loadSkin(QString image, bool save)
{
    //载入皮肤，自动预存3种尺寸
    skin = QPixmap(image);
    skinLeft = skin.scaledToWidth(360, Qt::SmoothTransformation);
    skinFull = skin.scaledToWidth(710, Qt::SmoothTransformation);
    aspectRatio = (double)skin.height() / skin.width();
    if (save)
        saveSkinData();//保存皮肤
    this->update();//重绘界面
}

void ShadowPlayer::UpdateTime()
{
    //Timer周期
    //音量计更新
    ui->leftLevel->setValue(LOWORD(player->getLevel()));
    ui->rightLevel->setValue(HIWORD(player->getLevel()));
    ui->leftLevel->update();//不显示文字进度时，不会立即重绘，故手动重绘
    ui->rightLevel->update();

    //计算对数音量
    //公式：dB=20log(X/V0)
    //最大音量：20log(32768/32768)=20log(1)=0dB
    //最小音量：20log(0) = 20(-∞) ……
    //最小音量此处以-60dB截止（参考foobar2000

    //后来参考Audition发现公式有误……
    //未修改，去掉20即可（音量计较短……还是不修改比较好）
    double ldB = 20*log((double)ui->leftLevel->value()/32768);
    double rdB = 20*log((double)ui->rightLevel->value()/32768);
    if (ldB < -60)
        ldB = -60;
    if (rdB < -60)
        rdB = -60;
    ui->leftdB->setValue(ldB);
    ui->rightdB->setValue(rdB);
    ui->leftdB->update();
    ui->rightdB->update();

    if (skinPos == 3)
        this->update();

    ui->curTimeLabel->setText(player->getCurTime());
    if (isPlaySliderPress == false)//当进度条按下时，不自动滚动
        ui->playSlider->setSliderPosition(player->getPos());

    this->UpdateFFT();

    //播放结束后相关操作
    if (!player->isPlaying())
    {
        switch (playMode) {
        case 0:
            //单曲播放
            //倒放模式会有BUG
            playing = false;
            ui->playButton->setIcon(QIcon(":/icon/ICO/Play.png"));
            ui->playButton->setToolTip("播放（Ctrl+F2）");//停止播放
            taskbarProgress->hide();
            taskbarButton->setOverlayIcon(QIcon(":/icon/ICO/Stop.png"));
            playToolButton->setIcon(QIcon(":/icon/ICO/Play.png"));
            playToolButton->setToolTip("播放");
            break;
        case 1:
            //单曲循环
            if (playing)
            {
                //如果没有暂停、停止
                player->stop();
                player->play();
                //再次播放
            }
            break;
        case 2:
            //顺序播放
            if (playing)
            {
                QString nextFile = playList->next(false);//跳至下一曲
                if (nextFile == "stop")//如果已经没有下一曲
                {
                    playing = false;
                    ui->playButton->setIcon(QIcon(":/icon/ICO/Play.png"));
                    ui->playButton->setToolTip("播放（Ctrl+F2）");//停止播放
                    taskbarProgress->hide();
                    taskbarButton->setOverlayIcon(QIcon(":/icon/ICO/Stop.png"));
                    playToolButton->setIcon(QIcon(":/icon/ICO/Play.png"));
                    playToolButton->setToolTip("播放");
                } else {
                    loadFile(nextFile);//如果有，继续播放
                }
            }
            break;
        case 3:
            //列表循环
            if (playing)
            {
                loadFile(playList->next(true));
            }
            break;
        case 4:
            //随机播放
            if (playing)
            {
                QTime time;
                time = QTime::currentTime();
                qsrand(time.msec() + time.second() * 1000);
                int index = qrand() % playList->getLength();
                //随机数
                loadFile(playList->playIndex(index));
            }
        default:
            break;
        }
    }
}

void ShadowPlayer::UpdateLrc()
{
    //刷新歌词时间
    lyrics->updateTime(player->getCurTimeMS(), player->getTotalTimeMS());
    double pos = 0;
    double curTimePos = lyrics->getTimePos(player->getCurTimeMS());
    //改变歌词文本
    ui->lrcLabel_1->setText(lyrics->getLrcString(-3));
    ui->lrcLabel_2->setText(lyrics->getLrcString(-2));
    ui->lrcLabel_3->setText(lyrics->getLrcString(-1));
    ui->lrcLabel_4->setText(lyrics->getLrcString(0));
    ui->lrcLabel_5->setText(lyrics->getLrcString(1));
    ui->lrcLabel_6->setText(lyrics->getLrcString(2));
    ui->lrcLabel_7->setText(lyrics->getLrcString(3));

    ui->lrcLabel_1->setToolTip(ui->lrcLabel_1->text());
    ui->lrcLabel_2->setToolTip(ui->lrcLabel_2->text());
    ui->lrcLabel_3->setToolTip(ui->lrcLabel_3->text());
    ui->lrcLabel_4->setToolTip(ui->lrcLabel_4->text());
    ui->lrcLabel_5->setToolTip(ui->lrcLabel_5->text());
    ui->lrcLabel_6->setToolTip(ui->lrcLabel_6->text());
    ui->lrcLabel_7->setToolTip(ui->lrcLabel_7->text());

    //根据歌词位置，滚动歌词
    if (curTimePos >= 0.2 && curTimePos <= 0.8)
    {
        pos = 0.5;
    } else if (curTimePos < 0.2) {
        pos = curTimePos * 2.5;//0~0.5
    } else if (curTimePos > 0.8) {
        pos = (curTimePos - 0.8) * 2.5 + 0.5;//0.5~1
    }
    ui->lrcLabel_1->setGeometry(10, 35 - 20 * pos, 311, 16);
    //ui->lrcLabel_1->setStyleSheet(QString("color: rgba(0, 0, 0, %1)").arg(245 - 235 * curTimePos));
    ui->lrcLabel_2->setGeometry(10, 55 - 20 * pos, 311, 16);
    ui->lrcLabel_3->setGeometry(10, 75 - 20 * pos, 311, 16);
    ui->lrcLabel_4->setGeometry(10, 95 - 20 * pos, 311, 16);
    ui->lrcLabel_5->setGeometry(10, 115 - 20 * pos, 311, 16);
    ui->lrcLabel_6->setGeometry(10, 135 - 20 * pos, 311, 16);
    ui->lrcLabel_7->setGeometry(10, 155 - 20 * pos, 311, 16);
    //ui->lrcLabel_7->setStyleSheet(QString("color: rgba(0, 0, 0, %1)").arg(235 * curTimePos + 10));
}

void ShadowPlayer::showCoverPic(QString filePath){
    QFileInfo fileinfo(filePath);
    QString path = fileinfo.path();
    if (spID3::loadPictureData(filePath.toLocal8Bit().data()))
    {
        QByteArray picData((const char*)spID3::getPictureDataPtr(), spID3::getPictureLength());
        ui->coverLabel->setPixmap(QPixmap::fromImage(QImage::fromData(picData)));
        spID3::freePictureData();
    } else if (spFLAC::loadPictureData(filePath.toLocal8Bit().data()))
    {
        QByteArray picData((const char*)spFLAC::getPictureDataPtr(), spFLAC::getPictureLength());
        ui->coverLabel->setPixmap(QPixmap::fromImage(QImage::fromData(picData)));
        spFLAC::freePictureData();
    }
    else if (QFileInfo(path + "/cover.jpg").exists())
        ui->coverLabel->setPixmap(QPixmap(path + "/cover.jpg"));
    else if (QFileInfo(path + "/cover.jpeg").exists())
        ui->coverLabel->setPixmap(QPixmap(path + "/cover.jpeg"));
    else if (QFileInfo(path + "/cover.png").exists())
        ui->coverLabel->setPixmap(QPixmap(path + "/cover.png"));
    else if (QFileInfo(path + "/cover.gif").exists())
        ui->coverLabel->setPixmap(QPixmap(path + "/cover.gif"));
    else
        ui->coverLabel->setPixmap(QPixmap(":image/image/ShadowPlayer.png"));
}

void ShadowPlayer::on_openButton_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this, QString::fromUtf8("打开"),0,QString::fromUtf8("音频文件 (*.mp3 *.mp2 *.mp1 *.wav *.aiff *.ogg *.ape *.mp4 *.m4a *.m4v *.aac *.alac *.tta *.flac *.wma *.wv)"));
    int newIndex = playList->getLength();
    int length = files.length();
    if (!files.isEmpty())
    {
        for (int i = 0 ;i < length; i++)
        {
            playList->add(files[i]);
        }
        if (playList->getLength() > newIndex) //如果添加了文件
        {
            loadFile(playList->playIndex(newIndex));//上次的列表长度就是此次播放的索引
        }
    }
}

void ShadowPlayer::on_playButton_clicked()
{
    if (!playing)
    {
        player->play();//启动播放

        if (player->isPlaying())//检测播放状态
        {
            playing = true;
            ui->playButton->setIcon(QIcon(":/icon/ICO/Pause.png"));
            ui->playButton->setToolTip("暂停（Ctrl+F2）");
            taskbarProgress->show();
            taskbarProgress->resume();
            taskbarButton->setOverlayIcon(QIcon(":/icon/ICO/Play.png"));
            playToolButton->setIcon(QIcon(":/icon/ICO/Pause.png"));
            playToolButton->setToolTip("暂停");
        } else {
            //不播放可能是文件没载入，尝试载入
            if(playList->getLength() > 0)
                loadFile(playList->playIndex(playList->getIndex()));
        }
    }
    else
    {
        player->pause();
        playing = false;
        ui->playButton->setIcon(QIcon(":/icon/ICO/Play.png"));
        ui->playButton->setToolTip("播放（Ctrl+F2）");
        taskbarProgress->show();
        taskbarProgress->pause();
        taskbarButton->setOverlayIcon(QIcon(":/icon/ICO/Pause.png"));
        playToolButton->setIcon(QIcon(":/icon/ICO/Play.png"));
        playToolButton->setToolTip("播放");
    }
}

void ShadowPlayer::on_stopButton_clicked()
{
    player->stop();
    playing = false;
    ui->playButton->setIcon(QIcon(":/icon/ICO/Play.png"));
    ui->playButton->setToolTip("播放（Ctrl+F2）");
    taskbarProgress->hide();
    taskbarButton->setOverlayIcon(QIcon(":/icon/ICO/Stop.png"));
    playToolButton->setIcon(QIcon(":/icon/ICO/Play.png"));
    playToolButton->setToolTip("播放");
}

void ShadowPlayer::on_volSlider_valueChanged(int value)
{
    player->setVol(value);
    isMute = false;//解除静音
    ui->muteButton->setIcon(QIcon(":/icon/ICO/Vol.png"));
}


void ShadowPlayer::on_muteButton_clicked()
{
    if (isMute == false)
    {
        //程序有改动，现在setValue即可变更静音状态
        //如果没有静音
        lastVol = ui->volSlider->value();
        ui->volSlider->setValue(0);
        //player->setVol(0);//已无需
        ui->muteButton->setIcon(QIcon(":/icon/ICO/Mute.png"));
        isMute = true;//必须先更改value才能静音，否则自动解除静音
    }
    else
    {
        ui->volSlider->setValue(lastVol);
        //player->setVol(lastVol);//已无需
        ui->muteButton->setIcon(QIcon(":/icon/ICO/Vol.png"));
        isMute = false;//无用代码，setValue已经解除静音
    }
}

float ShadowPlayer::arraySUM(int start, int end, float *array)
{
    float sum = 0;
    for(int i=start; i<=end; i++)
    {
        sum += array[i];
    }
    return sum;
}

void ShadowPlayer::fullZero(int length, float *array)
{
    for(int i = 0; i < length; i++)
    {
        array[i] = 0;
    }
}

void ShadowPlayer::UpdateFFT()
{
    //更新频谱分析（之前）
    //由于2 * x ^ 29 = 2047
    //故x ≈ 1.269
    //起始值 = 2 * pow(1.27,index);
    //终止值 = start * 1.27;

    //现又改为大约50Hz开始
    //为了优化代码……循环
    //x ≈ 1.23048

    if (player->isPlaying())
    {
        if (ui->leftLevel->value() > 6 || ui->rightLevel->value() > 6)
            player->getFFT(fftData);
        else
            fullZero(2048, fftData);

        double start = 5;
        for (int i = 0; i < 29 ; i++)
        {
            double end = start * 1.23048;
            ui->FFTGroupBox->fftBarValue[i] = sqrt(arraySUM((int)start,(int)end, fftData));
            start = end;
        }

        //限制最大最小值
        ui->FFTGroupBox->cutValue();
        //下滑峰值
        ui->FFTGroupBox->peakSlideDown();
        //重绘
        ui->FFTGroupBox->update();
    } else {
        ui->FFTGroupBox->peakSlideDown();
        ui->FFTGroupBox->update();
    }
}

void ShadowPlayer::on_playSlider_sliderPressed()
{
    isPlaySliderPress = true;
}

void ShadowPlayer::on_playSlider_sliderReleased()
{
    isPlaySliderPress = false;
    player->setPos(ui->playSlider->sliderPosition());
}

void ShadowPlayer::on_resetFreqButton_clicked()
{
    ui->freqSlider->setSliderPosition(0);
    player->setFreq(oriFreq);
}

//设置EQ效果
void ShadowPlayer::applyEQ()
{
    player->setEQ(0, ui->eqSlider_1->value());
    player->setEQ(1, ui->eqSlider_2->value());
    player->setEQ(2, ui->eqSlider_3->value());
    player->setEQ(3, ui->eqSlider_4->value());
    player->setEQ(4, ui->eqSlider_5->value());
    player->setEQ(5, ui->eqSlider_6->value());
    player->setEQ(6, ui->eqSlider_7->value());
    player->setEQ(7, ui->eqSlider_8->value());
    player->setEQ(8, ui->eqSlider_9->value());
    player->setEQ(9, ui->eqSlider_10->value());
}

//窗口拖动相关
void ShadowPlayer::mousePressEvent(QMouseEvent *event)
{
    //如果点击到控件上的话，pos无法取得数值，窗口会瞬移……
    //于是稍作修改
    if (event->button() == Qt::LeftButton && event->x() > 10) //只处理左键+x坐标大于10的情况
    {
        //下一行解决点击控件瞬移BUG
        pos = event->pos();
        clickOnFrame = true;//点击到窗口框架上才会变成true，改动之后新增功能：判断点击是否在边框右侧
        event->accept();
    } else if (event->button() == Qt::LeftButton && event->x() <= 10) {
        clickOnLeft = true;//点击了左侧，开启背景图位置调整
    }

}

void ShadowPlayer::mouseReleaseEvent(QMouseEvent *)
{
    clickOnFrame = false;//弹起鼠标按键时，恢复
    clickOnLeft = false;
}

void ShadowPlayer::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && clickOnFrame && event->x() > 10)
    {
        //如果鼠标按住左键在窗口右侧移动，并且点击在窗口上，则移动窗口
        move(event->globalPos() - pos);
        event->accept();
    } else if (event->buttons() & Qt::LeftButton && clickOnLeft && event->y() >= 0 && event->y() <= 400) {
        //如果鼠标点击到左侧小于10像素的区域，并移动，则调整背景图位置
        //已限制调节范围
        skinDrawPos = (double)event->y() / 400; //400：窗口高度
        skinPos = -1;//-1在switch语句中不存在此case
        this->update();
    }
}

//展开窗口右侧部分
void ShadowPlayer::on_extraButton_clicked()
{
    if (this->geometry().width() < 535)
    {
        sizeSlideAnimation->stop();
        sizeSlideAnimation->setStartValue(QRect(this->geometry().x(), this->geometry().y(), 360, 402));
        sizeSlideAnimation->setEndValue(QRect(this->geometry().x() - 175, this->geometry().y(), 710, 400));
        sizeSlideAnimation->start();
        ui->extraButton->setText("←");
    } else {
        sizeSlideAnimation->stop();
        sizeSlideAnimation->setStartValue(QRect(this->geometry().x(), this->geometry().y(), 710, 402));
        sizeSlideAnimation->setEndValue(QRect(this->geometry().x() + 175, this->geometry().y(), 360, 400));
        sizeSlideAnimation->start();
        ui->extraButton->setText("→");
    }
}

void ShadowPlayer::on_closeButton_clicked()
{
    fadeOutAnimation->start();//动画完成之后，程序将退出
    //qApp->quit();//退出
}

void ShadowPlayer::closeEvent(QCloseEvent *)
{
    qApp->exit();//主窗口关闭后，关闭程序
}

void ShadowPlayer::on_setSkinButton_clicked()
{
    QString image = QFileDialog::getOpenFileName(this, QString::fromUtf8("选择皮肤"),0,QString::fromUtf8("图像文件 (*.bmp *.jpg *.jpeg *.png *.gif)"));
    if(!image.isEmpty())
    {
        loadSkin(image);
    }
}

void ShadowPlayer::on_miniSizeButton_clicked()
{
    this->showMinimized();
}

//切换播放模式
void ShadowPlayer::on_playModeButton_clicked()
{
    playMode = ++playMode % 5;//切换模式
    //改了上述语句后编译时总有警告……
    //其实构造函数已经初始化这个变量啦><
    //这里没有问题哦~
    switch (playMode) {
    case 0:
        ui->playModeButton->setIcon(QIcon(":/icon/ICO/Single.png"));
        ui->playModeButton->setToolTip("单曲播放");
        break;
    case 1:
        ui->playModeButton->setIcon(QIcon(":/icon/ICO/Repeat.png"));
        ui->playModeButton->setToolTip("单曲循环");
        break;
    case 2:
        ui->playModeButton->setIcon(QIcon(":/icon/ICO/Order.png"));
        ui->playModeButton->setToolTip("顺序播放");
        break;
    case 3:
        ui->playModeButton->setIcon(QIcon(":/icon/ICO/AllRepeat.png"));
        ui->playModeButton->setToolTip("列表循环");
        break;
    case 4:
        ui->playModeButton->setIcon(QIcon(":/icon/ICO/Shuffle.png"));
        ui->playModeButton->setToolTip("随机播放");
        break;
    default:
        break;
    }
    QToolTip::showText(QCursor::pos(), ui->playModeButton->toolTip());//立即显示工具提示
}

void ShadowPlayer::on_showDskLrcButton_clicked()
{
    if(lb->isVisible())
        lb->hide();
    else
        lb->show();
}

void ShadowPlayer::on_loadLrcButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, QString::fromUtf8("载入歌词"),0,QString::fromUtf8("LRC歌词 (*.lrc)"));
    if(!file.isEmpty())
        lyrics->resolve(file, true);
}

void ShadowPlayer::on_playSlider_valueChanged(int value)
{
    //防止进度发生变更时，递归触发setPos
    if (qAbs(player->getPos() - value) > 2)
    {
        player->setPos(value);
    }
}

void ShadowPlayer::on_freqSlider_valueChanged(int value)
{
    player->setFreq(oriFreq + (oriFreq * value * 0.0001));
    ui->textLabel1->setText(QString::fromUtf8("回放速度 ") + "(x" + QString::number(value * 0.0001 + 1) + ")");
}

//预设的EQ方案（总觉得用文本文件存储好点……）
void ShadowPlayer::on_eqComboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        //还原
        ui->eqSlider_1->setSliderPosition(0);
        ui->eqSlider_2->setSliderPosition(0);
        ui->eqSlider_3->setSliderPosition(0);
        ui->eqSlider_4->setSliderPosition(0);
        ui->eqSlider_5->setSliderPosition(0);
        ui->eqSlider_6->setSliderPosition(0);
        ui->eqSlider_7->setSliderPosition(0);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(0);
        ui->eqSlider_10->setSliderPosition(0);
        applyEQ();
        break;
    case 1:
        //流行
        ui->eqSlider_1->setSliderPosition(3);
        ui->eqSlider_2->setSliderPosition(1);
        ui->eqSlider_3->setSliderPosition(0);
        ui->eqSlider_4->setSliderPosition(-2);
        ui->eqSlider_5->setSliderPosition(-4);
        ui->eqSlider_6->setSliderPosition(-4);
        ui->eqSlider_7->setSliderPosition(-2);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(1);
        ui->eqSlider_10->setSliderPosition(2);
        applyEQ();
        break;
    case 2:
        //摇滚
        ui->eqSlider_1->setSliderPosition(-2);
        ui->eqSlider_2->setSliderPosition(0);
        ui->eqSlider_3->setSliderPosition(2);
        ui->eqSlider_4->setSliderPosition(4);
        ui->eqSlider_5->setSliderPosition(-2);
        ui->eqSlider_6->setSliderPosition(-2);
        ui->eqSlider_7->setSliderPosition(0);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(4);
        ui->eqSlider_10->setSliderPosition(4);
        applyEQ();
        break;
    case 3:
        //电子
        ui->eqSlider_1->setSliderPosition(-6);
        ui->eqSlider_2->setSliderPosition(1);
        ui->eqSlider_3->setSliderPosition(4);
        ui->eqSlider_4->setSliderPosition(-2);
        ui->eqSlider_5->setSliderPosition(-2);
        ui->eqSlider_6->setSliderPosition(-4);
        ui->eqSlider_7->setSliderPosition(0);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(6);
        ui->eqSlider_10->setSliderPosition(6);
        applyEQ();
        break;
    case 4:
        //古典
        ui->eqSlider_1->setSliderPosition(0);
        ui->eqSlider_2->setSliderPosition(8);
        ui->eqSlider_3->setSliderPosition(8);
        ui->eqSlider_4->setSliderPosition(4);
        ui->eqSlider_5->setSliderPosition(0);
        ui->eqSlider_6->setSliderPosition(0);
        ui->eqSlider_7->setSliderPosition(0);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(2);
        ui->eqSlider_10->setSliderPosition(2);
        applyEQ();
        break;
    case 5:
        //金属
        ui->eqSlider_1->setSliderPosition(-6);
        ui->eqSlider_2->setSliderPosition(0);
        ui->eqSlider_3->setSliderPosition(0);
        ui->eqSlider_4->setSliderPosition(0);
        ui->eqSlider_5->setSliderPosition(0);
        ui->eqSlider_6->setSliderPosition(0);
        ui->eqSlider_7->setSliderPosition(4);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(4);
        ui->eqSlider_10->setSliderPosition(0);
        applyEQ();
        break;
    case 6:
        //舞曲
        ui->eqSlider_1->setSliderPosition(-2);
        ui->eqSlider_2->setSliderPosition(3);
        ui->eqSlider_3->setSliderPosition(4);
        ui->eqSlider_4->setSliderPosition(1);
        ui->eqSlider_5->setSliderPosition(-2);
        ui->eqSlider_6->setSliderPosition(-2);
        ui->eqSlider_7->setSliderPosition(0);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(4);
        ui->eqSlider_10->setSliderPosition(4);
        applyEQ();
        break;
    case 7:
        //乡村
        ui->eqSlider_1->setSliderPosition(-2);
        ui->eqSlider_2->setSliderPosition(0);
        ui->eqSlider_3->setSliderPosition(0);
        ui->eqSlider_4->setSliderPosition(2);
        ui->eqSlider_5->setSliderPosition(2);
        ui->eqSlider_6->setSliderPosition(0);
        ui->eqSlider_7->setSliderPosition(0);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(4);
        ui->eqSlider_10->setSliderPosition(4);
        applyEQ();
        break;
    case 8:
        //爵士
        ui->eqSlider_1->setSliderPosition(0);
        ui->eqSlider_2->setSliderPosition(0);
        ui->eqSlider_3->setSliderPosition(0);
        ui->eqSlider_4->setSliderPosition(4);
        ui->eqSlider_5->setSliderPosition(4);
        ui->eqSlider_6->setSliderPosition(4);
        ui->eqSlider_7->setSliderPosition(0);
        ui->eqSlider_8->setSliderPosition(2);
        ui->eqSlider_9->setSliderPosition(3);
        ui->eqSlider_10->setSliderPosition(4);
        applyEQ();
        break;
    case 9:
        //布鲁斯
        ui->eqSlider_1->setSliderPosition(-2);
        ui->eqSlider_2->setSliderPosition(0);
        ui->eqSlider_3->setSliderPosition(2);
        ui->eqSlider_4->setSliderPosition(1);
        ui->eqSlider_5->setSliderPosition(0);
        ui->eqSlider_6->setSliderPosition(0);
        ui->eqSlider_7->setSliderPosition(0);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(-2);
        ui->eqSlider_10->setSliderPosition(-4);
        applyEQ();
        break;
    case 10:
        //怀旧
        ui->eqSlider_1->setSliderPosition(-4);
        ui->eqSlider_2->setSliderPosition(0);
        ui->eqSlider_3->setSliderPosition(2);
        ui->eqSlider_4->setSliderPosition(1);
        ui->eqSlider_5->setSliderPosition(0);
        ui->eqSlider_6->setSliderPosition(0);
        ui->eqSlider_7->setSliderPosition(0);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(-4);
        ui->eqSlider_10->setSliderPosition(-6);
        applyEQ();
        break;
    case 11:
        //歌剧
        ui->eqSlider_1->setSliderPosition(0);
        ui->eqSlider_2->setSliderPosition(0);
        ui->eqSlider_3->setSliderPosition(0);
        ui->eqSlider_4->setSliderPosition(4);
        ui->eqSlider_5->setSliderPosition(5);
        ui->eqSlider_6->setSliderPosition(3);
        ui->eqSlider_7->setSliderPosition(6);
        ui->eqSlider_8->setSliderPosition(3);
        ui->eqSlider_9->setSliderPosition(0);
        ui->eqSlider_10->setSliderPosition(0);
        applyEQ();
        break;
    case 12:
        //语音
        ui->eqSlider_1->setSliderPosition(-4);
        ui->eqSlider_2->setSliderPosition(0);
        ui->eqSlider_3->setSliderPosition(2);
        ui->eqSlider_4->setSliderPosition(0);
        ui->eqSlider_5->setSliderPosition(0);
        ui->eqSlider_6->setSliderPosition(0);
        ui->eqSlider_7->setSliderPosition(0);
        ui->eqSlider_8->setSliderPosition(0);
        ui->eqSlider_9->setSliderPosition(-4);
        ui->eqSlider_10->setSliderPosition(-6);
        applyEQ();
        break;
    default:
        break;
    }
}

void ShadowPlayer::addToListAndPlay(QList<QUrl> files)
{
    int newIndex = playList->getLength();
    int length = files.length();
    if (!files.isEmpty())
    {
        for (int i = 0 ;i < length; i++)
        {
            playList->add(files[i].toLocalFile());
        }
        if (playList->getLength() > newIndex) //如果添加了文件
            loadFile(playList->playIndex(newIndex));
    }
}

void ShadowPlayer::addToListAndPlay(QStringList files)
{
    int newIndex = playList->getLength();
    int length = files.length();
    if (length > 0)
    {
        for (int i = 0; i < length; i++)
        {
            playList->add(files[i]);
        }
        if (playList->getLength() > newIndex) //如果添加了文件
            loadFile(playList->playIndex(newIndex));
    }
}

void ShadowPlayer::addToListAndPlay(QString file)
{
    int newIndex = playList->getLength();
    playList->add(file);
    if (playList->getLength() > newIndex) //如果添加了文件
        loadFile(playList->playIndex(newIndex));
}

void ShadowPlayer::on_playPreButton_clicked()
{
    if (playMode == 3)
    {
        //列表循环
        loadFile(playList->previous(true));
    } else if (playMode == 4) {
        if (!playList->isEmpty())//如果列表非空
        {
            //随机播放
            QTime time;
            time = QTime::currentTime();
            qsrand(time.msec() + time.second() * 1000);
            int index = qrand() % playList->getLength();
            //随机数
            loadFile(playList->playIndex(index));
        }
    } else {
        //正常情况
        loadFile(playList->previous(false));
    }
}

void ShadowPlayer::on_playNextButton_clicked()
{
    if (playMode == 3)
    {
        //列表循环
        loadFile(playList->next(true));
    } else if (playMode == 4) {
        if (!playList->isEmpty())//如果列表非空
        {
            //随机播放
            QTime time;
            time = QTime::currentTime();
            qsrand(time.msec() + time.second() * 1000);
            int index = qrand() % playList->getLength();
            //随机数
            loadFile(playList->playIndex(index));
        }
    } else {
        loadFile(playList->next(false));
    }
}

void ShadowPlayer::on_playListButton_clicked()
{
    //如果右侧没有展开，则展开窗口
    if (width() < 370)
        on_extraButton_clicked();

    //判断动画是否不在播放
    if (playListHideAnimation->state() != QAbstractAnimation::Running &&
            playListShowAnimation->state() != QAbstractAnimation::Running)
    {
        if (ui->playerListArea->height() < 190)
        {
            eqHideAnimation->stop();
            lyricsHideAnimation->stop();
            playListShowAnimation->stop();
            eqHideAnimation->start();
            lyricsHideAnimation->start();
            playListShowAnimation->start();
        } else {
            if (width() > 370) //如果窗口已展开，则收起播放列表。否则什么都不做
            {
                eqShowAnimation->stop();
                lyricsShowAnimation->stop();
                playListHideAnimation->stop();
                eqShowAnimation->start();
                lyricsShowAnimation->start();
                playListHideAnimation->start();
            }
        }
    }
}

void ShadowPlayer::callFromPlayList()
{
    //播放列表的播放请求
    loadFile(playList->getCurFile());
    //双击时触发的事件，已经无需选中，直接播放。
}

//倒放按钮事件
void ShadowPlayer::on_reverseButton_clicked()
{
    if (isReverse)
    {
        isReverse = false;
        ui->reverseButton->setText("正放");
        player->setReverse(false);
    } else {
        isReverse = true;
        ui->reverseButton->setText("倒放");
        player->setReverse(true);
    }
}

//混响效果调节
void ShadowPlayer::on_reverbDial_valueChanged(int value)
{
    player->updateReverb(value);
}

void ShadowPlayer::physicsSetting()
{
    double temp = 0;
    bool ok = false;

    temp = QInputDialog::getDouble(0, "加速度", "重力加速度\n【参数均为比率，1 = 频谱条总长度】", ui->FFTGroupBox->acc, 0, 2147483647, 3, &ok);
    if (ok)
    {
        ui->FFTGroupBox->acc = temp;
    }

    temp = QInputDialog::getDouble(0, "最大下落速度", "物体下落的最大速度", ui->FFTGroupBox->maxSpeed, 0, 2147483647, 3, &ok);
    if (ok)
    {
        ui->FFTGroupBox->maxSpeed = temp;
    }

    temp = QInputDialog::getDouble(0, "基准速度", "总体速度、力的倍增值\n此数值影响弹力、抛力\n最初版本用于下落速度，当时无物理效果\n【谨慎修改】", ui->FFTGroupBox->speed, 0, 2147483647, 3, &ok);
    if (ok)
    {
        ui->FFTGroupBox->speed = temp;
    }

    temp = QInputDialog::getDouble(0, "抛力倍增系数", "频谱条对物体的抛力强度倍增系数\n抛力将按此倍数加倍", ui->FFTGroupBox->forceD, 0, 2147483647, 3, &ok);
    if (ok)
    {
        ui->FFTGroupBox->forceD = temp;
    }

    temp = QInputDialog::getDouble(0, "弹力系数", "落地后动能保留量\n物体落地时，部分动能转化为声音和热能\n此参数决定碰撞后剩余的动能百分比\n【注：由于算法BUG，每次下落都将损失最后一帧离地高度的势能，所以动能一直损耗】\n因为没做空气阻力，所以应该让动能损耗更多\n1 = 完全反弹", ui->FFTGroupBox->elasticCoefficient, 0, 2147483647, 3, &ok);
    if (ok)
    {
        ui->FFTGroupBox->elasticCoefficient = temp;
    }

    temp = QInputDialog::getDouble(0, "弹力阀值", "当物体一帧下落的百分比（视为可穿透）小于该值时\n不计算弹力", ui->FFTGroupBox->minElasticStep, 0, 2147483647, 3, &ok);
    if (ok)
    {
        ui->FFTGroupBox->minElasticStep = temp;
    }
}

//频谱分析预设物理开
void ShadowPlayer::enableFFTPhysics()
{
    ui->FFTGroupBox->acc = 0.35;
    ui->FFTGroupBox->maxSpeed = 9;
    ui->FFTGroupBox->speed = 0.025;
    ui->FFTGroupBox->forceD = 6;
    ui->FFTGroupBox->elasticCoefficient = 0.6;
    ui->FFTGroupBox->minElasticStep = 0.02;
}

//频谱分析预设物理关
void ShadowPlayer::disableFFTPhysics()
{
    ui->FFTGroupBox->acc = 0.15;
    ui->FFTGroupBox->maxSpeed = 2;
    ui->FFTGroupBox->speed = 0.025;
    ui->FFTGroupBox->forceD = 0;
    ui->FFTGroupBox->elasticCoefficient = 0;
    ui->FFTGroupBox->minElasticStep = 0.02;
}

void ShadowPlayer::resizeEvent(QResizeEvent *)
{
    ui->extraButton->setGeometry(width() - 20, 0, 20, 20);
    ui->closeButton->setGeometry(width() - 65, 0, 40, 20);
    ui->miniSizeButton->setGeometry(width() - 90, 0, 25, 20);
    ui->setSkinButton->setGeometry(width() - 115, 0, 25, 20);
}

void ShadowPlayer::showDeveloperInfo()
{
    QMessageBox::information(0, "暗影播放器", "版本:正式版\n作者：暗影夜光（陈XX）\n\n联系方式：\nQQ:617274873\n版权没有，自由使用");
}

void ShadowPlayer::saveConfig()
{
    QFile file(QCoreApplication::applicationDirPath() + "/config.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream << (quint32)0x61727480
           << ui->freqSlider->value()
           << ui->volSlider->value()
           << isMute
           << ui->reverbDial->value()
           << playMode
           << skinMode
           << skinPos
           << skinDrawPos
           << ui->eqComboBox->currentIndex()
           << ui->eqEnableCheckBox->isChecked()
           << ui->eqSlider_1->value()
           << ui->eqSlider_2->value()
           << ui->eqSlider_3->value()
           << ui->eqSlider_4->value()
           << ui->eqSlider_5->value()
           << ui->eqSlider_6->value()
           << ui->eqSlider_7->value()
           << ui->eqSlider_8->value()
           << ui->eqSlider_9->value()
           << ui->eqSlider_10->value()
           << lb->isVisible();
    file.close();
}

void ShadowPlayer::loadConfig()
{
    QFile file(QCoreApplication::applicationDirPath() + "/config.dat");
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    quint32 magic;
    stream >> magic;
    if (magic == 0x61727480)
    {
        int dataInt = 0;
        stream >> dataInt;
        ui->freqSlider->setValue(dataInt);
        stream >> dataInt;
        ui->volSlider->setValue(dataInt);
        bool dataBool = false;
        stream >> dataBool;
        this->isMute = dataBool;
        if (isMute)
        {
            ui->muteButton->setIcon(QIcon(":/icon/ICO/Mute.png"));
        } else {
            ui->muteButton->setIcon(QIcon(":/icon/ICO/Vol.png"));
        }
        stream >> dataInt;
        ui->reverbDial->setValue(dataInt);
        stream >> this->playMode;
        switch (playMode) {
        case 0:
            ui->playModeButton->setIcon(QIcon(":/icon/ICO/Single.png"));
            ui->playModeButton->setToolTip("单曲播放");
            break;
        case 1:
            ui->playModeButton->setIcon(QIcon(":/icon/ICO/Repeat.png"));
            ui->playModeButton->setToolTip("单曲循环");
            break;
        case 2:
            ui->playModeButton->setIcon(QIcon(":/icon/ICO/Order.png"));
            ui->playModeButton->setToolTip("顺序播放");
            break;
        case 3:
            ui->playModeButton->setIcon(QIcon(":/icon/ICO/AllRepeat.png"));
            ui->playModeButton->setToolTip("列表循环");
            break;
        case 4:
            ui->playModeButton->setIcon(QIcon(":/icon/ICO/Shuffle.png"));
            ui->playModeButton->setToolTip("随机播放");
            break;
        default:
            break;
        }
        stream >> this->skinMode;
        stream >> this->skinPos;
        stream >> this->skinDrawPos;
        stream >> dataInt;
        ui->eqComboBox->setCurrentIndex(dataInt);
        stream >> dataBool;
        ui->eqEnableCheckBox->setChecked(dataBool);
        stream >> dataInt;
        ui->eqSlider_1->setValue(dataInt);
        stream >> dataInt;
        ui->eqSlider_2->setValue(dataInt);
        stream >> dataInt;
        ui->eqSlider_3->setValue(dataInt);
        stream >> dataInt;
        ui->eqSlider_4->setValue(dataInt);
        stream >> dataInt;
        ui->eqSlider_5->setValue(dataInt);
        stream >> dataInt;
        ui->eqSlider_6->setValue(dataInt);
        stream >> dataInt;
        ui->eqSlider_7->setValue(dataInt);
        stream >> dataInt;
        ui->eqSlider_8->setValue(dataInt);
        stream >> dataInt;
        ui->eqSlider_9->setValue(dataInt);
        stream >> dataInt;
        ui->eqSlider_10->setValue(dataInt);
        stream >> dataBool;
        lb->setVisible(dataBool);
    }
    file.close();
}

void ShadowPlayer::saveSkinData()
{
    QFile file(QCoreApplication::applicationDirPath() + "/skin.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream << (quint32)0x61727481 << skin;
    file.close();
}

void ShadowPlayer::loadSkinData()
{
    QFile file(QCoreApplication::applicationDirPath() + "/skin.dat");
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    quint32 magic;
    stream >> magic;
    if (magic == 0x61727481)
    {
        stream >> this->skin;
        skinLeft = skin.scaledToWidth(360, Qt::SmoothTransformation);
        skinFull = skin.scaledToWidth(710, Qt::SmoothTransformation);
        aspectRatio = (double)skin.height() / skin.width();
    }
    file.close();
}

bool ShadowPlayer::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);
    MSG* msg = reinterpret_cast<MSG*>(message);
    if (msg->message == WM_COPYDATA){
        COPYDATASTRUCT *p = reinterpret_cast<COPYDATASTRUCT*>(msg->lParam);
        addToListAndPlay(QString::fromUtf8((LPCSTR)(p->lpData)));
        return true;
    }
    return false;
}

//设置Win7任务栏按钮对应窗口
void ShadowPlayer::setTaskbarButtonWindow()
{
    taskbarButton->setWindow(windowHandle());
    thumbnailToolBar->setWindow(windowHandle());
}

void ShadowPlayer::on_eqEnableCheckBox_clicked(bool checked)
{
    if (checked)
    {
        player->eqReady();//初始化均衡器
        applyEQ();//应用当前值
    } else {
        player->disableEQ();//移除均衡器
    }
}
