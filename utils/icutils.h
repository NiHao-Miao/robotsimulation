#ifndef ICUTILS_H
#define ICUTILS_H

#include <QObject>



#define PARAM_IN
#define PARAM_OUT

class ICUtils : public QObject
{
    Q_OBJECT
public:
    explicit ICUtils(QObject *parent = nullptr);


    static bool copyFile(const QString &sourceFile,
                                         const QString &targetFile);

    static bool copyRobotModelFiles(const QString& name, const QStringList &sourceFiles);

    static QString copyLuaScriptToFile(const QString& name, const QString &source);


    ///
    /// \brief openUrl  用浏览器打开
    /// \param url
    ///
    static void openUrl(const QString& url);

    ///
    /// \brief openExplorer  打开资源管理器
    /// \param path
    ///
    static void openExplorer(const QString& path);

    static QString getFileName(const QString &filter, QString defaultDir);
    static QStringList getFileNames(const QString &filter,
                                      QString defaultDir);
    static QString getFolderName();
    static QString getSystemUsername();
    static QString getFileNameWithExtension(const QString &strFilePath);
    static QStringList getFolderFileNames(const QStringList &filter) ;
    static bool folderIsExist(const QString &strFolder);
    static bool fileIsExist(const QString &strFile);
    static void deleteDirectory(const QString &path);
    static bool isWebOk();
    static bool ipLive(const QString &ip, int port, int timeout=2000);
    static QStringList getPathFileList(const QString &path,
                                         const QStringList &filters,
                                         bool absolute_path);

    static QStringList getPathFileList(const QString &path,
                                         const QString &filter,
                                         bool absolute_path) ;
    static bool stringFormatInput(PARAM_IN const QString& input, PARAM_IN const QString& format ,PARAM_OUT QVariantList* param);

    static QString modelNameToPath(const QString& name);
    static bool copyRobot(const QString &path);
    static bool copyDirectory(const QString &sourcePath, const QString &destinationPath);
signals:

};

#endif // ICUTILS_H
