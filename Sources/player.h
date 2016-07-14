#ifndef PLAYER_H
#define PLAYER_H
#include "bass.h"
#include "bass_ape.h"
#include "bass_aac.h"
#include "bass_alac.h"
#include "bass_tta.h"
#include "bassflac.h"
#include "basswma.h"
#include "basswv.h"
#include "tags.h"
#include "bass_fx.h"
#include <QString>
#include <QFileInfo>

class Player
{
public:
    Player();
    ~Player();
    bool devInit();
    QString getTags();
    void setVol(int vol);
    int getVol();
    QString openFile(QString fileName);
    void play();//播放
    void pause();//暂停
    void stop();//停止并跳回头部
    int getPos();//取得播放位置
    void setPos(int pos);//改变播放位置
    int getBitRate();//计算比特率
    void getFFT(float *array);
    bool isPlaying();
    int getFreq();
    void setFreq(float freq);
    void eqReady();
    void disableEQ();
    void setEQ(int id, int gain);
    QString getNowPlayInfo();//取得媒体参数信息描述
    QString getTotalTime();//取得用于显示的总时间
    QString getCurTime();//取得用于显示的当前时间
    int getCurTimeMS();//取得当前播放时间的毫秒数（整数）
    int getTotalTimeMS();//取得总播放时间的毫秒数（整数）
    double getCurTimeSec();
    double getTotalTimeSec();
    DWORD getLevel();
    QString getFileTotalTime(QString fileName);//计算文件时间
    double getFileSecond(QString fileName);
    void setReverse(bool isEnable);
    void updateReverb(int value);//更新混响效果，参数取值范围：0~20
    void setJumpPoint(double timeFrom, double timeTo);
    void removeJumpPoint();

private:
    HSTREAM nowPlay;//播放流句柄
    HFX eqFX;//10段均衡器效果
    HFX reverbFX;//混响效果
    bool playNextEnable;//暂时无用
    HSYNC jumping;
};
#endif // PLAYER_H
