# ShadowPlayer
一个基于Qt5.3.1和BASS库的简易播放器。

## Windows二进制程序下载
请访问 http://pan.baidu.com/s/1ntOeCf3

## 分支版本
* [BLumia修改版](https://github.com/Blumia/ShadowPlayer-BLumia)

## 功能特点
* 无损音频播放
* 可作为Windows文件关联来使用      
（防多开 + 进程间通信实现在一个窗口打开多个文件）
* 变速播放
* 倒放
* 播放列表，支持拖放添加，支持批量操作
* 均衡器和混响
* 滚动歌词
* 桌面歌词，可设置字体和阴影样式
* 可更换播放器背景图片
* 全局热键控制播放
* 带有 __物理效果__ 的频谱分析
* ID3、Flac专辑封面显示
* 无序播放
* OSD显示当前播放曲目
* 任务栏播放进度及状态显示
* 任务栏控制按钮
* 拖放操作，拖入文件自动加载（自动识别歌词、背景图片、音频文件）
* 桌面歌词过长时，自动滚动歌词

## 代码里能够拿出去用的东西，均为作者实现（或绘制）
* ID3/FLAC专辑封面提取
* 带物理效果的频谱显示控件（频谱需要自己计算）
* 横向点击定位进度条（稍作修改可以支持纵向）
* 带阴影的QLabel控件（支持三种阴影模式，无阴影、描边和斜45投影）
* 控件图标

本播放器的进度条可以直接点击定位。

## 截图
![主界面](https://github.com/ShadowPower/ShadowPlayer/raw/master/Screenshots/1.png)

![播放状态](https://github.com/ShadowPower/ShadowPlayer/raw/master/Screenshots/2.png)

![右键菜单](https://github.com/ShadowPower/ShadowPlayer/raw/master/Screenshots/3.png)

![播放列表](https://github.com/ShadowPower/ShadowPlayer/raw/master/Screenshots/4.png)

![更换背景图片以及桌面歌词](https://github.com/ShadowPower/ShadowPlayer/raw/master/Screenshots/5.png)

![透明背景](https://github.com/ShadowPower/ShadowPlayer/raw/master/Screenshots/6.png)

![桌面歌词字体设置](https://github.com/ShadowPower/ShadowPlayer/raw/master/Screenshots/7.png)
