#ifndef SHADOWPLAYER_H
#define SHADOWPLAYER_H
#include <QMainWindow>
#include "player.h"
#include "lyrics.h"
#include "lrcbar.h"
#include "playlist.h"
#include "osd.h"
#include "bass.h"
#include <QtWidgets>
#include <QtCore>
#include <QtGui>
#include <QtWinExtras>
#include <math.h>
#include <spslider.h>
#include <MyGlobalShortCut/MyGlobalShortCut.h>
#include <omp.h>

namespace Ui {
class ShadowPlayer;
}

class ShadowPlayer : public QMainWindow
{
    Q_OBJECT

public:
    explicit ShadowPlayer(QWidget *parent = 0);
    ~ShadowPlayer();
    void loadFile(QString file);
    void loadSkin(QString image, bool save = true);
    int cutInt(int i);
    void addToListAndPlay(QList<QUrl> files);
    void addToListAndPlay(QStringList files);
    void addToListAndPlay(QString file);

private slots:
    void UpdateTime();
    void UpdateLrc();
    void callFromPlayList();//接受播放列表的请求，启动播放
    void on_openButton_clicked();
    void on_playButton_clicked();
    void on_stopButton_clicked();
    void on_volSlider_valueChanged(int value);
    void on_muteButton_clicked();
    void on_playSlider_sliderPressed();
    void on_playSlider_sliderReleased();
    void on_resetFreqButton_clicked();
    void applyEQ();//应用均衡器效果
    void on_extraButton_clicked();
    void on_closeButton_clicked();
    void on_setSkinButton_clicked();
    void loadDefaultSkin();
    void fixSkinSizeLeft();
    void fixSkinSizeFull();
    void originalSkinSize();
    void autoSkinSize();
    void dynamicSkinSize();
    void skinOnTop();
    void skinOnCenter();
    void skinOnButtom();
    void skinDisable();
    void physicsSetting();
    void enableFFTPhysics();
    void disableFFTPhysics();
    void on_miniSizeButton_clicked();
    void on_playModeButton_clicked();
    void on_loadLrcButton_clicked();
    void on_playSlider_valueChanged(int value);
    void on_freqSlider_valueChanged(int value);
    void on_eqComboBox_currentIndexChanged(int index);
    void on_playPreButton_clicked();
    void on_playNextButton_clicked();
    void on_playListButton_clicked();
    void on_reverseButton_clicked();
    void showDeveloperInfo();
    void on_reverbDial_valueChanged(int value);
    void setTaskbarButtonWindow();

    void on_eqEnableCheckBox_clicked(bool checked);

public slots:
    void on_showDskLrcButton_clicked();

private:
    Ui::ShadowPlayer *ui;

    float arraySUM(int start, int end, float *array);//求数组平均数
    void UpdateFFT();
    void showCoverPic(QString filePath);
    void infoLabelAnimation();
    void drawFFTBar(QWidget *parent, int x, int y, int width, int height, double percent);
    void drawFFTBarPeak(QWidget *parent, int x, int y, int width, int height, double percent);
    void fullZero(int length, float *array);//数组填0
    void saveConfig();
    void loadConfig();
    void saveSkinData();
    void loadSkinData();

    Player *player;
    PlayList *playList;
    Lyrics *lyrics;
    OSD *osd;
    QTimer *timer;
    QTimer *lrcTimer;
    LrcBar *lb;//歌词窗口
    bool isPlaySliderPress;
    bool isMute;//是否静音
    int lastVol;//静音前的音量值
    float fftData[2048];//fft数据
    double fftBarValue[29];//存放频谱分析条的值
    double fftBarPeakValue[29];//存放频谱分析顶峰的值
    int oriFreq;//存放歌曲初始采样率
    bool playing;//播放？（防止歌曲无法停止，Timer判断，用于循环播放，下一曲等，false不会启动播放）

    QPoint pos;//用于窗口拖动，存储鼠标坐标
    bool clickOnFrame;//是否点击到了窗口上，用于解决瞬移BUG
    bool clickOnLeft;//用于调节背景图位置

    QPixmap skin;//用作皮肤的背景图片
    QPixmap skinLeft;
    QPixmap skinFull;
    double aspectRatio;//图片宽高比，用于动态缩放
    int skinMode;//皮肤模式 0.原始 1.左侧 2.全屏 3.自动  4.动态缩放
    int playMode;//播放模式 0.单曲播放 1.单曲循环 2.顺序播放 3.列表循环 4.随机播放
    int skinPos;//预设背景图片位置 0.显示顶端 1.显示中间 2.显示底部
    double skinDrawPos;//绘制背景图片时，使用的百分比位置 0 = 顶部 1 = 底部
    bool isReverse;//是否倒放

    //Windows NT 6.x 相关
    QWinTaskbarButton *taskbarButton;
    QWinTaskbarProgress *taskbarProgress;

    QWinThumbnailToolBar *thumbnailToolBar;
    QWinThumbnailToolButton *playToolButton;
    QWinThumbnailToolButton *stopToolButton;
    QWinThumbnailToolButton *backwardToolButton;
    QWinThumbnailToolButton *forwardToolButton;

    //各种动画
    QPropertyAnimation *sizeSlideAnimation;
    QPropertyAnimation *fadeInAnimation;
    QPropertyAnimation *tagAnimation;
    QPropertyAnimation *mediaInfoAnimation;
    QPropertyAnimation *coverAnimation;
    QPropertyAnimation *fadeOutAnimation;

    QPropertyAnimation *eqHideAnimation;
    QPropertyAnimation *eqShowAnimation;
    QPropertyAnimation *lyricsHideAnimation;
    QPropertyAnimation *lyricsShowAnimation;
    QPropertyAnimation *playListHideAnimation;
    QPropertyAnimation *playListShowAnimation;

    //渐变背景
    QLinearGradient bgLinearGradient;

protected:
    void dragEnterEvent(QDragEnterEvent *event);//拖放相关
    void dropEvent(QDropEvent *event);

    void paintEvent(QPaintEvent *);//界面绘图

    void mousePressEvent(QMouseEvent *event);//窗体拖动相关
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void contextMenuEvent(QContextMenuEvent *);//右键菜单
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent *);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
};

#endif // SHADOWPLAYER_H
