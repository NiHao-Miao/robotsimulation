#include "autoupdate.h"
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QMessageBox>
#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QCoreApplication>
#include <QApplication>

static QString ZENTAO_USER_NAME = "pangshubo";
static QString ZENTAO_USER_PASSWORD = "88888888";
static QString ZENTAO_URL_BASE = "http://rd.hc-system.com:9003";
static QString ZENTAO_URL_API_BASE = ZENTAO_URL_BASE+"/zentao/api.php/v1";
static QString ZENTAO_PRODUCT_ID = "1486";
static QString ZENTAO_PROJECT_ID = "2970";


AutoUpdate::AutoUpdate(QObject *parent):QObject(parent),max_history_count(1)
{
    manager = new QNetworkAccessManager(this);
    bug_manager =  new QNetworkAccessManager(this);
    settings = new QSettings("./config/update.ini", QSettings::IniFormat);

    download_reply = nullptr;
    download_dialog = new QDialog();
    download_dialog->setWindowTitle("文件下载中");
    progress_bar = new QProgressBar(download_dialog);
    download_label = new QLabel(download_dialog);
    download_dialog->resize(300,60);
    progress_bar->resize(280,20);
    download_label->resize(280,30);
    progress_bar->move(10,10);
    download_label->move(10,progress_bar->height()+20);
    auto_check_update_timer.setInterval(14400000);
    auto_check_update_timer.setSingleShot(false);
    connect(&auto_check_update_timer,&QTimer::timeout,this,[this](){
        this->checkUpdate();
    });

    mysql_db=QSqlDatabase::addDatabase("QMYSQL");
    mysql_db.setPort(23306);
    mysql_db.setHostName("43.138.136.232");
    mysql_db.setUserName("ota_update");
    mysql_db.setPassword("ota_update");
    mysql_db.setDatabaseName("my_database");
    mysql_db.setConnectOptions("MYSQL_OPT_RECONNECT=1");//断网重联
    if(mysql_db.open())
    {
        qDebug()<<"connect success";
        APP_OTA_INFO info;
        QApplication* app = qobject_cast<QApplication*>(QCoreApplication::instance());
        QString appName = app->applicationName();
        bool ret = getOtaInfo(appName,&info);
        if(ret){
            ZENTAO_USER_NAME = info.app_username;
            ZENTAO_USER_PASSWORD = info.app_password;
            ZENTAO_URL_BASE = info.app_url;
            ZENTAO_URL_API_BASE = ZENTAO_URL_BASE+"/zentao/api.php/v1";
            ZENTAO_PRODUCT_ID = info.product_id;
            ZENTAO_PROJECT_ID = info.project_id;
        }
    }
    else {
        qDebug()<<"connect failed!"<<mysql_db.lastError();
    }
}

AutoUpdate::~AutoUpdate()
{
    delete  settings;
    delete  download_dialog;
}

AutoUpdate *AutoUpdate::Instance()
{
    static AutoUpdate auto_update;
    return &auto_update;
}

bool AutoUpdate::getOtaInfo(const QString& app_name, AutoUpdate::APP_OTA_INFO* info)
{
    if(!mysql_db.open()){
        return false;
    }

    // 创建查询对象
    QSqlQuery query(mysql_db);

    // 准备 SQL 查询，查找 app_name 为 'RobotSimulation' 的记录
    query.prepare("SELECT * FROM tb_OTAList WHERE app_name = :appName");
    query.bindValue(":appName", app_name);

    // 执行查询
    if (!query.exec()) {
        qDebug() << "Query Execution Error:" << query.lastError().text();
        return false;
    }

    // 检索结果
    while (query.next()) {
        // 假设 tb_OTAList 表中有字段 id 和 app_name
        info->app_url = query.value("app_url").toString();    // 获取 app_url 字段
        info->app_name = query.value("app_name").toString(); // 获取 app_name 字段
        info->app_username = query.value("app_username").toString(); // 获取 app_username 字段
        info->app_password = query.value("app_password").toString(); // 获取 app_password 字段
        info->project_id = query.value("project_id").toString(); // 获取 project_id 字段
        info->product_id = query.value("product_id").toString(); // 获取 product_id 字段

        // 打印结果
        qDebug() << "App_url:" << info->app_url << ", App Name:" << app_name<<info->app_username<<info->app_password<<info->project_id<<info->product_id;
    }
    return true;
}


QVariant AutoUpdate::getCustomSetting(QString dir,QVariant defaultValue){
    QString name = QString("custom/%1").arg(dir);
    return settings->value(name,defaultValue);
}

void AutoUpdate::setCustomSetting(QString dir,QVariant value){
    QString name = QString("custom/%1").arg(dir);
    settings->setValue(name,value);
}

void AutoUpdate::checkUpdate(bool show)
{
    isShow = show;
    QNetworkReply* reply = login(manager);
    if(reply->error() == QNetworkReply::NoError){
        connect(manager,&QNetworkAccessManager::finished,this,&AutoUpdate::loginPostBack);
    }
    else{
        if(isShow){
            QMessageBox::warning(nullptr,"检查更新", QString("网络异常，无法连接服务器!\nUrl:%1\nError:%2").arg(reply->url().toString()).arg(reply->error()));
        }
    }
}

void AutoUpdate::createBug(const QString &title, const QString &data, const QString &account,bool show)
{
    isShow = show;
    bug.account = account.toUtf8();
    bug.data = data.toUtf8();
    bug.title = title.toUtf8();
    QNetworkReply* reply = login(bug_manager);
    if(reply->error() == QNetworkReply::NoError){
        connect(bug_manager,&QNetworkAccessManager::finished,this,&AutoUpdate::createBugPostBack);
    }
    else{
        if(isShow){
            QMessageBox::warning(nullptr,"反馈", QString("网络异常，无法连接服务器!\nUrl:%1\nError:%2").arg(reply->url().toString()).arg(reply->error()));
        }
    }
}

QString AutoUpdate::getHistoryVersionList() const
{
    return version_list;
}

void AutoUpdate::getVersionList()
{
    QUrl ZENTAO_URL = QUrl(ZENTAO_URL_API_BASE+"/projects/"+ZENTAO_PROJECT_ID+"/builds");

    // 构造请求
    QNetworkRequest request;
    request.setUrl(ZENTAO_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Token",token.toUtf8());
    // 发送请求
    manager->get(request);
    connect(manager,&QNetworkAccessManager::finished,this,&AutoUpdate::getVersionListPostBack);
}

void AutoUpdate::getVersionInfo(uint32_t id)
{
    QUrl ZENTAO_URL = QUrl(ZENTAO_URL_API_BASE+QString("/builds/%1").arg(id));

    // 构造请求
    QNetworkRequest request;
    request.setUrl(ZENTAO_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Token",token.toUtf8());
    // 发送请求
    manager->get(request);
    connect(manager,&QNetworkAccessManager::finished,this,&AutoUpdate::getVersionInfoPostBack);
}

void AutoUpdate::downloadFile(const QString &url, const QString &filename)
{
    QUrl ZENTAO_URL = QUrl(ZENTAO_URL_BASE+url);
    if(filename.endsWith(".exe")){
        this->download_file_name = filename;
    }

    // 构造请求
    QNetworkRequest request;
    request.setUrl(ZENTAO_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Token",token.toUtf8());
    // 发送请求
    download_reply = manager->get(request);
    connect(download_reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(updateDownloadProgress(qint64, qint64)));
    connect(manager,&QNetworkAccessManager::finished,this,&AutoUpdate::downloadFilePostBack);
}

void AutoUpdate::startUpdate()
{
    if(this->download_files.size() == 0){
        return;
    }
    download_reply = nullptr;
    download_total = this->download_files.size();
    this->downloadFile(download_files.at(0).url,download_files.at(0).filename);
}

void AutoUpdate::createBug()
{
    QUrl ZENTAO_URL = QUrl(ZENTAO_URL_API_BASE+"/products/"+ZENTAO_PRODUCT_ID+"/bugs");
    // 构造请求
    QNetworkRequest request;
    request.setUrl(ZENTAO_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Token",token.toUtf8());

    // Json数据
    QJsonObject json;
    json["product"]=ZENTAO_PRODUCT_ID;
    json.insert("title", bug.title);
    json.insert("severity", 4);
    json.insert("pri", 4);
    json.insert("type", "codeerror");
    json.insert("steps", bug.data);
    json.insert("keywords", QString("version:%1,user:%2").arg(SW_VER).arg(bug.account));
    //    json.insert("browser", bug.account);
    QJsonArray json1;
    json1.append("trunk");
    json.insert("openedBuild", json1);

    QJsonDocument document;
    document.setObject(json);
    QByteArray dataArray = document.toJson(QJsonDocument::Compact);

    // 发送请求
    bug_manager->post(request,dataArray);
    connect(bug_manager,&QNetworkAccessManager::finished,this,&AutoUpdate::createPostBack1);
}

bool AutoUpdate::compareVersion(const QString &cur_version, const QString &version)
{
    int32_t ret = 0;
    QStringList v1 = cur_version.split(".");
    QStringList v2 = version.split(".");
    int i = 0;
    while (1) {
        int32_t v1_number = 0;  //< 当前版本号
        int32_t v2_number = 0;  //< 检查版本号
        if(v1.size() > i){
            v1_number = v1[i].toUInt();
        }
        else{
            v1_number = -1;
        }
        if(v2.size() > i){
            v2_number = v2[i].toUInt();
        }
        else {
            v2_number = -1;
        }
        if(v2_number > v1_number){
            return true;
        }
        if(v2_number < v1_number){
            return false;
        }
        if(v2_number == -1 && v1_number == -1){
            break;
        }
        i++;
    }
    return false;
}

int32_t AutoUpdate::stringToInt(const QString &str)
{
    QString tmp;
    for(int j = 0; j < str.length(); j++)
    {
        if(str[j] > '0' && str[j] < '9'){
            tmp.append(str[j]);
        }
    }
    return tmp.toInt();
}

void AutoUpdate::addCheckUpdatePath(const QString &update_name, const QString &user_name, const QString &user_password, const QString &server_url_base, const QString &server_api_base, const QString &product_id, const QString &project_id)
{
    check_update_list.push_back({update_name,user_name,user_password,server_url_base,server_api_base,product_id,project_id});
}

void AutoUpdate::loginPostBack(QNetworkReply *reply)
{
    disconnect(manager,&QNetworkAccessManager::finished,this,&AutoUpdate::loginPostBack);
    if (reply->error() != QNetworkReply::NoError)
    {
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        qDebug()<<reply->error();
        if(isShow){
            QMessageBox::warning(nullptr,"检查更新", QString("网络异常，无法连接服务器!\nUrl:%1\nError:%2\n").arg(request.url().toString()).arg(reply->error()));
        }
        return;
    }
    // 获取响应信息
    QByteArray bytes = reply->readAll();

    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(bytes, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << QStringLiteral("解析Json失败:")<<bytes;
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        if(isShow){
            QMessageBox::warning(nullptr,"检查更新", QString("数据格式错误，无法解析数据!\nUrl:%1\n").arg(request.url().toString()));
        }
        return;
    }

    // 解析Json
    if (doucment.isObject())
    {
        QJsonObject obj = doucment.object();
        QJsonValue value;
        if (obj.contains("token"))
        {
            value = obj.take("token");
            if (value.isString())
            {
                QString data = value.toString();
                token = data;
                qDebug()<<"token" << data;
            }
            this->getVersionList();
        }
    }

}

void AutoUpdate::getVersionListPostBack(QNetworkReply *reply)
{
    disconnect(manager,&QNetworkAccessManager::finished,this,&AutoUpdate::getVersionListPostBack);
    if (reply->error() != QNetworkReply::NoError)
    {
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        qDebug()<<"getVersionListPostBack error:"<<reply->error();
        if(isShow){
            QMessageBox::warning(nullptr,"检查更新", QString("网络异常，无法连接服务器!\nUrl:%1\nError:%2").arg(request.url().toString()).arg(reply->error()));
        }
        return;
    }
    // 获取响应信息
    QByteArray bytes = reply->readAll();

    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(bytes, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << QStringLiteral("解析Json失败:")<<bytes;
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        if(isShow){
            QMessageBox::warning(nullptr,"检查更新", QString("数据格式错误，无法解析数据!\nUrl:%1").arg(request.url().toString()));
        }
        return;
    }
    // 解析Json
    if (doucment.isObject())
    {
        QJsonObject obj = doucment.object();
        QJsonValue value;
        if (obj.contains("builds"))
        {
            value = obj.take("builds");
            if (value.isArray())
            {
                QJsonArray data = value.toArray();
                if(data.size()){
                    build_list.clear();
                    version_list.clear();
                    int size = data.size()>max_history_count?max_history_count:data.size();
                    for(int i = 0;i<size;i++){
                        uint32_t version_id = data[i].toObject()["id"].toInt();
                        QString version_name = data[i].toObject()["name"].toString();
                        build_list.append(version_id);
                        if(i == 0){
                            qDebug() << QStringLiteral("最新版本:")<<version_id<<version_name;
                            this->getVersionInfo(version_id);
                        }
                    }
                }
                else{
                    if(isShow){
                        QMessageBox::information(nullptr,"检查更新","已是最新版本!");
                    }
                }
            }
        }
    }
}

void AutoUpdate::getVersionInfoPostBack(QNetworkReply *reply)
{
    disconnect(manager,&QNetworkAccessManager::finished,this,&AutoUpdate::getVersionInfoPostBack);
    QByteArray bytes;
    QJsonParseError jsonError;
    QJsonDocument doucment ;

    build_list.removeAt(0);
    if (reply->error() != QNetworkReply::NoError)
    {
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        qDebug()<<"getVersionListPostBack error:"<<reply->error();
        if(isShow){
            QMessageBox::warning(nullptr,"检查更新", QString("网络异常，无法连接服务器!\nUrl:%1\nError:%2").arg(request.url().toString()).arg(reply->error()));
        }
        goto STEP1;
    }
    // 获取响应信息
    bytes = reply->readAll();


    doucment = QJsonDocument::fromJson(bytes, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << QStringLiteral("解析Json失败:")<<bytes;
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        if(isShow){
            QMessageBox::warning(nullptr,"检查更新", QString("数据格式错误，无法解析数据!\nUrl:%1").arg(request.url().toString()));
        }
        goto STEP1;
    }
    // 解析Json
    if (doucment.isObject())
    {
        QJsonObject obj = doucment.object();
        QJsonValue value;
        QString name = obj["name"].toString();
        QString date = obj["date"].toString();
        QString desc = obj["desc"].toString();
        qDebug()<<"当前版本："<<SW_VER;
        qDebug()<<"版本名称："<<name;
        qDebug()<<"创建日期："<<date;
        qDebug()<<"更新日志："<<desc;
        desc = desc.replace("<p>","");
        desc = desc.replace("</p>","");

        QString str ="版本名称:"+name + '\n' +"创建日期："+ date+"\n" + "更新内容："+ desc+"\n\n---------------------------\n";
        version_list.append(str);
        if(!isShow){
            if(getCustomSetting("updateversion","").toString() == name){
                goto STEP1;
            }
        }
        bool isUpdate = compareVersion(SW_VER,name);
        if(isUpdate){
            qDebug()<<"需要更新 "<<name;
            int res = 0;
            if(name.endsWith("-f")){
                res = QMessageBox::question(
                    nullptr,                                  //指定父窗体
                    "检测更新",                                //指定标题
                    QString("发现新版本:%1(当前版本:%4)\n更新日期:%2\n更新日志:%3\n").arg(name).arg(date).arg(desc).arg(SW_VER), //显示的文本
                    "升级","","",0
                    );
            }
            else{
                res = QMessageBox::question(
                    nullptr,                                  //指定父窗体
                    "检测更新",                                //指定标题
                    QString("发现新版本:%1(当前版本:%4)\n更新日期:%2\n更新日志:%3\n").arg(name).arg(date).arg(desc).arg(SW_VER), //显示的文本
                    "升级","稍后升级","不再提醒",0
                    );
            }
            switch (res) {
            case 0:
                if(obj.contains("files")){
                    QJsonObject files = obj["files"].toObject();
                    auto it  = files.begin();
                    while(it != files.end()){
                        QJsonObject sub = it->toObject();

                        if(sub.contains("title")){
                            QString title = sub["title"].toString();
                            QString path = sub["webPath"].toString();

                            download_files.append({path,title});
                        }
                        ++it;
                    }
                    if(files.size()){
                        qDebug()<<"发现升级包:"<<download_files.size();
                        this->startUpdate();
                        return;
                    }
                }
                break;
            case 2:
                //< 不再提醒
                setCustomSetting("updateversion",name);
                break;
            default:
                break;
            }
        }
        else{
            if(isShow){
                QMessageBox::information(nullptr,"检查更新",QString("%1已是最新版本!").arg(SW_VER));
            }
        }
    }
STEP1:
    isShow = false;
    //< 查找下一个版本
    if(build_list.size()){
        this->getVersionInfo(build_list[0]);
    }
    else{
//        qDebug()<<" 历史版本信息: "<<version_list;
    }
}

void AutoUpdate::downloadFilePostBack(QNetworkReply *reply)
{
    connect(manager,&QNetworkAccessManager::finished,this,&AutoUpdate::downloadFilePostBack);
    if (reply->error() != QNetworkReply::NoError)
    {
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        qDebug()<<"getVersionListPostBack error:"<<reply->error();
        if(isShow){
            QMessageBox::warning(nullptr,"检查更新", QString("网络异常，无法连接服务器!Url:%1Error:%2").arg(request.url().toString()).arg(reply->error()));
        }
        return;
    }
    // 获取响应信息
    QByteArray bytes = reply->readAll();
    QDir dir;
    if(!dir.exists("update")){
        dir.mkdir("update");
    }
    dir.cd("update");
    QString file_name = dir.absoluteFilePath(this->download_files.at(0).filename);
    QFile file(file_name);
    if(file.open(QFile::ReadWrite)){
        file.write(bytes);
        file.close();
    }
    qDebug()<<"升级包下载完成"<<this->download_files.at(0).filename;
    this->download_files.removeAt(0);
    if(this->download_files.size() == 0){
        qDebug()<<"准备更新";
        int res = QMessageBox::information(nullptr,"检查更新","升级包下载完成,点击确定安装更新包!\n"+dir.absoluteFilePath(this->download_file_name),QString("确定"));
        if (res == 0) {
            //< 准备升级
            QProcess *p=new QProcess();
            p->start("\""+dir.absoluteFilePath(this->download_file_name)+"\"");
            exit(0);
        }
    }
    else{
        this->startUpdate();
    }
}

void AutoUpdate::createBugPostBack(QNetworkReply *reply)
{
    disconnect(bug_manager,&QNetworkAccessManager::finished,this,&AutoUpdate::createBugPostBack);
    if (reply->error() != QNetworkReply::NoError)
    {
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        qDebug()<<reply->error();
        if(isShow){
            QMessageBox::warning(nullptr,"检查更新", QString("网络异常，无法连接服务器!\nUrl:%1\nError:%2\n").arg(request.url().toString()).arg(reply->error()));
        }
        return;
    }
    // 获取响应信息
    QByteArray bytes = reply->readAll();

    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(bytes, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << QStringLiteral("解析Json失败:")<<bytes;
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        if(isShow){
            QMessageBox::warning(nullptr,"检查更新", QString("数据格式错误，无法解析数据!\nUrl:%1\n").arg(request.url().toString()));
        }
        return;
    }

    // 解析Json
    if (doucment.isObject())
    {
        QJsonObject obj = doucment.object();
        QJsonValue value;
        if (obj.contains("token"))
        {
            value = obj.take("token");
            if (value.isString())
            {
                QString data = value.toString();
                token = data;
                qDebug()<<"token" << data;
            }
            qDebug()<<"createBug";
            this->createBug();
        }
    }

}

void AutoUpdate::createPostBack1(QNetworkReply *reply)
{
    disconnect(bug_manager,&QNetworkAccessManager::finished,this,&AutoUpdate::createPostBack1);
    qDebug()<<"createPostBack1";
    if (reply->error() != QNetworkReply::NoError)
    {
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        qDebug()<<reply->error();
        if(isShow)
            QMessageBox::warning(nullptr,"反馈建议", QString("网络异常，无法连接服务器!\nUrl:%1\nError:%2\n").arg(request.url().toString()).arg(reply->error()));
        return;
    }
    // 获取响应信息
    QByteArray bytes = reply->readAll();

    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(bytes, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << QStringLiteral("解析Json失败:")<<bytes;
        QNetworkRequest request = reply->request();
        qDebug() << QStringLiteral("request:")<<request.url();
        if(isShow)
            QMessageBox::warning(nullptr,"反馈建议", QString("数据格式错误，无法解析数据!\nUrl:%1\n").arg(request.url().toString()));
        return;
    }
    qDebug() <<" reply:" <<bytes;
    if(isShow)
        QMessageBox::information(nullptr,"反馈建议","已成功提交!");

}

void AutoUpdate::updateDownloadProgress(qint64 cur, qint64 total)
{
    qDebug()<<"updateDownloadProgress"<<cur<<total;
    progress_bar->setRange(0,100);
    progress_bar->setValue(cur*100/total);
    download_dialog->show();
    download_label->setText(QString("下载文件:%1/%2").arg(download_total - download_files.size()).arg(download_files.size()));
}

QNetworkReply* AutoUpdate::login(QNetworkAccessManager *m)
{
    QUrl ZENTAO_URL = QUrl(ZENTAO_URL_API_BASE+"/tokens");
    // Json数据
    QJsonObject json;
    json.insert("account", ZENTAO_USER_NAME);
    json.insert("password", ZENTAO_USER_PASSWORD);

    QJsonDocument document;
    document.setObject(json);
    QByteArray dataArray = document.toJson(QJsonDocument::Compact);

    // 构造请求
    QNetworkRequest request;
    request.setUrl(ZENTAO_URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    qDebug() << QStringLiteral("json:")<<dataArray;
    // 发送请求
    return m->post(request, dataArray);
}


