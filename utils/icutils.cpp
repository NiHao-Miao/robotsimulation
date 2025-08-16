#include "icutils.h"
#include <QDesktopServices>
#include <QProcess>
#include <QWidget>
#include <windows.h>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QTcpSocket>

ICUtils::ICUtils(QObject *parent) : QObject(parent)
{

}

bool ICUtils::copyFile(const QString &sourceFile,
                              const QString &targetFile) {
    bool ok;
    ok = QFile::copy(sourceFile, targetFile);
    // 将复制过去的文件只读属性取消
    ok = QFile::setPermissions(targetFile, QFile::WriteOwner);
    return ok;
}

bool ICUtils::copyRobotModelFiles(const QString &name, const QStringList &sourceFiles)
{
    QDir dir;
    if(!dir.cd("robot")){
        dir.mkdir("robot");
        dir.cd("robot");
    }
    dir.mkdir(name);
    dir.cd(name);
    for(int i = 0;i<sourceFiles.size();i++){
        QFileInfo file(sourceFiles.at(i));
        ICUtils::copyFile(sourceFiles.at(i),dir.absoluteFilePath(file.fileName()));
    }
    return true;
}

bool ICUtils::copyDirectory(const QString &sourcePath, const QString &destinationPath) {
    QDir sourceDir(sourcePath);
    QDir destinationDir(destinationPath);

    // 检查源目录是否存在
    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist:" << sourcePath;
        return false;
    }

    // 创建目标目录
    if (!destinationDir.exists()) {
        if (!destinationDir.mkpath(".")) {
            qDebug() << "Failed to create destination directory:" << destinationPath;
            return false;
        }
    }

    // 获取源目录中的所有文件和子目录
    QFileInfoList fileList = sourceDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    foreach (const QFileInfo &fileInfo, fileList) {
        QString newFilePath = destinationPath + "/" + fileInfo.fileName();

        if (fileInfo.isDir()) {
            // 递归拷贝子目录
            if (!copyDirectory(fileInfo.absoluteFilePath(), newFilePath)) {
                return false;
            }
        } else {
            // 拷贝文件
            if (!QFile::copy(fileInfo.absoluteFilePath(), newFilePath)) {
                qDebug() << "Failed to copy file:" << fileInfo.absoluteFilePath() << "to" << newFilePath;
                return false;
            }
        }
    }
    return true;
}

bool ICUtils::copyRobot(const QString &path)
{
    QFileInfo r(path);
    QString name = r.fileName();
    QDir dir;
    if(!dir.cd("robot")){
        dir.mkdir("robot");
        dir.cd("robot");
    }
    copyFile(path,dir.absoluteFilePath(name));
    copyDirectory(QDir(r.absolutePath()).absoluteFilePath(r.baseName()),dir.absoluteFilePath(r.baseName()));
    return true;
}


QString ICUtils::copyLuaScriptToFile(const QString &name, const QString &source)
{
    if(source.size()  ==0){
        return "";
    }
    QDir dir = QDir::temp();
    if(!dir.cd("robot")){
        dir.mkdir("robot");
        dir.cd("robot");
    }
//    dir.mkdir(name);
//    dir.cd(name);
    QFile file(dir.absoluteFilePath("main.lua"));
    if(file.open(QFile::ReadWrite)){
        file.resize(0);
        file.write(source.toUtf8());
        file.close();
        return dir.absoluteFilePath("main.lua");
    }
    return "";
}

QString ICUtils::getFileName(const QString &filter, QString defaultDir) {
    return QFileDialog::getOpenFileName(0, "选择文件", defaultDir, filter);
}

QStringList ICUtils::getFileNames(const QString &filter,
                                         QString defaultDir) {
    return QFileDialog::getOpenFileNames(0, "选择文件", defaultDir, filter);
}

QString ICUtils::getFolderName() {
    return QFileDialog::getExistingDirectory();
}

QString ICUtils::getSystemUsername()
{
    QString username;
#ifdef Q_OS_WIN
    QProcess process;
    process.start("cmd.exe", QStringList() << "/c" << "echo %username%");
    process.waitForFinished();
    username = QString::fromLocal8Bit(process.readAllStandardOutput().trimmed());
#elif defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    QProcess process;
    process.start("whoami");
    process.waitForFinished();
    username = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
#endif
    return username;
}


QString ICUtils::getFileNameWithExtension(const QString &strFilePath) {
    QFileInfo fileInfo(strFilePath);
    return fileInfo.fileName();
}

QStringList ICUtils::getFolderFileNames(const QStringList &filter) {
    QStringList fileList;
    QString strFolder = QFileDialog::getExistingDirectory();

    if (!strFolder.length() == 0) {
        QDir myFolder(strFolder);

        if (myFolder.exists()) {
            fileList = myFolder.entryList(filter);
        }
    }

    return fileList;
}

bool ICUtils::folderIsExist(const QString &strFolder) {
    QDir tempFolder(strFolder);
    return tempFolder.exists();
}

bool ICUtils::fileIsExist(const QString &strFile) {
    QFile tempFile(strFile);
    return tempFile.exists();
}


void ICUtils::deleteDirectory(const QString &path) {
    QDir dir(path);

    if (!dir.exists()) {
        qDebug()<<"目录不存在"<<path;
        return;
    }
    else{
         qDebug()<<"deleteDirectory"<<path;
    }

    dir.removeRecursively();
}

bool ICUtils::isWebOk() {
    // 能接通百度IP说明可以通外网
    return ipLive("43.138.136.232", 9003);
}


bool ICUtils::ipLive(const QString &ip, int port, int timeout) {
    QTcpSocket tcpClient;
    tcpClient.abort();
    tcpClient.connectToHost(ip, port);
    // 超时没有连接上则判断不在线
    return tcpClient.waitForConnected(timeout);
}

QStringList ICUtils::getPathFileList(const QString &path,
                                            const QStringList &filters,
                                            bool absolute_path) {
    QString current_path = QDir::currentPath();
    QString dir_path = path;
    if (path.indexOf(".") == 0 ||
        (path.indexOf("/") != 0 && path.indexOf("\\") != 0)) {
        dir_path = current_path + "/" + path;
    }
    QDir dir(dir_path);
    if (absolute_path == false) {
        return dir.entryList(filters, QDir::Files, QDir::Name);
    }
    QStringList path_list = dir.entryList(filters, QDir::Files, QDir::Name);
    QStringList result;
    QString end = "";
    if (path.right(0) != "/") {
        end = "/";
    }
    for (quint32 i = 0, len = path_list.length(); i < len; ++i) {
        result.append(dir_path + end + path_list.at(i));
    }
    return result;
}

QStringList ICUtils::getPathFileList(const QString &path,
                                            const QString &filter,
                                            bool absolute_path) {
    QString current_path = QDir::currentPath();
    QString dir_path = path;
//    if (path.indexOf(".") == 0 ||
//        (path.indexOf("/") != 0 && path.indexOf("\\") != 0)) {
//        dir_path = current_path + "/" + path;
//    }
    QDir dir(dir_path);
    if (!dir.exists()) {
        qDebug() << dir_path << "not exists";
        return QStringList();
    }
    QStringList filters;
    filters.append(filter);
    if (absolute_path == false) {
        return dir.entryList(filters, QDir::Files, QDir::Name);
    }
    QStringList path_list = dir.entryList(filters, QDir::Files, QDir::Name);
    //    QStringList path_list = dir.entryList();

    QStringList result;
    QString end = "";
    if (path.right(0) != "/") {
        end = "/";
    }
    for (quint32 i = 0, len = path_list.length(); i < len; ++i) {
        result.append(dir_path + end + path_list.at(i));
    }
    return result;
}





void ICUtils::openUrl(const QString &url) {
    if (url.size() == 0)
        return;
    // 打开外部浏览器
    QDesktopServices::openUrl(QUrl::fromLocalFile(url));

    // 获取当前活动窗口的ID
    QProcess process;
    process.start("xdotool getactivewindow");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    WId currentWindowId = output.trimmed().toULongLong(nullptr, 16);

    // 将浏览器窗口置顶
    QWidget *browserWindow = QWidget::find(currentWindowId);
    if (browserWindow) {
        Qt::WindowFlags flags = browserWindow->windowFlags();
        flags |= Qt::WindowStaysOnTopHint;
        browserWindow->setWindowFlags(flags);
        browserWindow->show();
    }
}

void ICUtils::openExplorer(const QString &path) {
    QString p = path;

    //将地址中的"/"替换为"\"，因为在Windows下使用的是"\"。
    p.replace("/", "\\");

    //打开文件管理器路径
    QProcess::startDetached("explorer " + p);
}


/// <summary>
/// 格式化输入字符串
/// </summary>
/// <param name="input">输入字符串</param>
/// <param name="format">字符串格式</param>
/// <param name="param">格式化后的参数</param>
/// <returns>是否正常格式化</returns>
bool ICUtils::stringFormatInput(PARAM_IN const QString& input, PARAM_IN const QString& format ,PARAM_OUT QVariantList* param) {
    bool error = false;
    int input_index = 0;
    int format_index = 0;
    while (input_index < input.size() && format_index < format.size())
    {
        if (input.at(input_index) == format.at(format_index)) {
            //< 字符匹配
            format_index++;
            input_index++;
        }
        else if(format.at(format_index) == '%') {
            if (format.at(format_index+1) == 'd') {
                //< 此刻开始 整数
                int i = 0;
                int symbol = 1;
                int ret = 0;
                while (true)
                {
                    if (input.at(input_index+i) >= '0' && input.at(input_index + i) <= '9') {
                        i++;
                    }
                    else if ((i == 0) && ((input.at(input_index) == '+') || (input.at(input_index) == '-'))) {
                        //< 符号位
                        i++;
                    }
                    else {
                        //< 不是数字 退出
                        break;
                    }
                    if((input_index+i)>= input.size()){
                        break;
                    }
                }
                QString num = input.mid(input_index,i);
                param->append(num.toInt());

                format_index +=2;
                input_index += i;
            }
            else if (format.at(format_index + 1) == 'f') {
                //< 此刻开始 整数
                int i = 0;
                int symbol = 1;
                float ret = 0;
                while (true)
                {
                    if ((input.at(input_index + i) >= '0' && input.at(input_index + i) <= '9')|| (input.at(input_index + i) == '.')) {
                        i++;
                    }
                    else if ((i == 0) && ((input.at(input_index) == '+') || (input.at(input_index) == '-'))) {
                        //< 符号位
                        i++;
                    }
                    else {
                        //< 不是数字 退出
                        break;
                    }
                    if ((input_index + i) >= input.size()) {
                        break;
                    }
                }
                QString num = input.mid(input_index,i);
                param->append(num.toDouble());

                format_index += 2;
                input_index += i;
            }
            else
            {
                error = true;
                break;
            }
        }
        else
        {
            error = true;
            break;
        }
    }
    if (error || (input_index != input.size() || format_index != format.size())){
        param->clear();
        return false;
    }
    return true;
}

QString ICUtils::modelNameToPath(const QString &name)
{
    QDir dir;
    if(!dir.cd("robot")){
        dir.mkdir("robot");
        dir.cd("robot");
    };
    return dir.absoluteFilePath(name+".hcrobot");
}
