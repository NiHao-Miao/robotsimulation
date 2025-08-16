#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include <QObject>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include <QSqlDatabase>

class QNetworkAccessManager;
class QNetworkReply;
class QSettings;

class AutoUpdate : public QObject
{
    Q_OBJECT
public:
    AutoUpdate(QObject* parent = nullptr);
    ~AutoUpdate();

    static AutoUpdate *Instance();

    ///
    /// \brief compareVersion   比较版本号
    /// \param cur_version
    /// \param version
    /// \return
    ///
    static bool compareVersion(const QString& cur_version,const QString& version);

    static int32_t stringToInt(const QString& str);

    typedef struct{
        QString update_name;
        QString user_name;
        QString user_password;
        QString server_url_base;
        QString server_api_base;
        QString product_id;
        QString project_id;
    }CHECK_UPDATE_DATA;


    typedef struct{
        QString app_name;
        QString app_username;
        QString app_password;
        QString app_url;
        QString project_id;
        QString product_id;
    }APP_OTA_INFO ;

    void addCheckUpdatePath(const QString& update_name,
                            const QString& user_name,
                            const QString& user_password,
                            const QString& server_url_base,
                            const QString& server_api_base,
                            const QString& product_id,
                            const QString& project_id);

public:
    ///
    /// \brief checkUpdate  检查更新
    /// \param show         是否显示检查更新过程
    ///
    void checkUpdate(bool show = false);

    ///
    /// \brief createBug    创建一个bug
    /// \param title        bug标题
    /// \param data         bug内容
    /// \param account      账户
    ///
    void createBug(const QString& title,const QString& data,const QString& account,bool show);

    QString getHistoryVersionList()  const;

    bool getOtaInfo(const QString &app_name, AutoUpdate::APP_OTA_INFO *info);

public  slots:
    void loginPostBack(QNetworkReply* reply);

    void getVersionListPostBack(QNetworkReply* reply);

    void getVersionInfoPostBack(QNetworkReply* reply);

    void downloadFilePostBack(QNetworkReply* reply);

    void createBugPostBack(QNetworkReply* reply);

    void createPostBack1(QNetworkReply* reply);


    void updateDownloadProgress(qint64, qint64);

private:
    QNetworkReply* login(QNetworkAccessManager *m);

private:
    void getVersionList();

    void getVersionInfo(uint32_t id);

    void downloadFile(const QString &url, const QString &filename);

    ///
    /// \brief startUpdate  开始更新
    ///
    void startUpdate();

private:
    void createBug();

private:
    QVariant getCustomSetting(QString dir,QVariant defaultValue);

    void setCustomSetting(QString dir,QVariant value);

    typedef struct{
        QString url;
        QString filename;
    }DOWNLOAD_TYPE;

    typedef struct{
        QString title;
        QString data;
        QString account;
    }BUG_TYPE;

private:
    QSqlDatabase mysql_db;
    QSettings* settings;
    QNetworkAccessManager * manager;
    QNetworkAccessManager * bug_manager;
    QString token;
    QString download_file_name;
    bool isShow;
    QList<DOWNLOAD_TYPE> download_files;
    BUG_TYPE bug;
    QList<CHECK_UPDATE_DATA> check_update_list;
    QNetworkReply* download_reply;
private:
    QProgressBar* progress_bar;
    QDialog* download_dialog;
    QLabel* download_label;
    int download_total;

private:
    QVector<quint32> build_list;
    QString version_list;
    QTimer auto_check_update_timer;

    uint32_t max_history_count;     //< 最大历史更新记录信息数量
};

#endif // AUTOUPDATE_H
