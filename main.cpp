#include "shadowplayer.h"
#include <QApplication>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QFile>
#include <windows.h>

#ifdef Q_OS_WIN32   //Windows
#include <windows.h>
bool checkOnly()
{
    HANDLE playerMutex = CreateMutex(NULL, FALSE, L"shadowplayer");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(playerMutex);
        playerMutex = NULL;
        return false;//存在
    } else {
        return true;//不存在
    }
}
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //二次启动，防止多开并载入文件
    if(!checkOnly())
    {
        QString file = QString::fromLocal8Bit(argv[1]);

        COPYDATASTRUCT cpd;
        cpd.dwData = 0;
        cpd.cbData = file.toUtf8().size() + 1;// + 1 是为了在末尾创建一个00（字符串终止）
        cpd.lpData = file.toUtf8().data();
        HWND hWnd = FindWindowW(L"Qt5QWindowIcon", L"ShadowPlayer");
        SendMessageW(hWnd, WM_COPYDATA, NULL, (LPARAM)&cpd);
        return -1;//退出
    }

    ShadowPlayer w;
    w.show();
    //直接使用此应用程序打开文件
    if(argc > 1)
    {
        QStringList fileList;
        for (int i = 1; i < argc; i++)
            fileList.append(QString::fromLocal8Bit(argv[i]));
        w.addToListAndPlay(fileList);
    }

    //首次运行
    QFile file(QCoreApplication::applicationDirPath() + "/FirstRun");
    if (file.exists())
    {
        file.remove();
        w.on_showDskLrcButton_clicked();
        w.addToListAndPlay(QCoreApplication::applicationDirPath() + "/DEMO/CONNECT~心的连接~.mp3");
        QMessageBox::information(0, "欢迎~", "您是第一次运行本程序\n点击右上角的箭头探索更多功能\n祝您玩得愉快", "好的");
    }

    return a.exec();
}
