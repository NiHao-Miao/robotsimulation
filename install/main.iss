; 该执行目录为 setup.iss 所在的目录，请注意拼接相对目录
#define MyAppId "{{8FDFC065-7DB0-4935-B9A8-756157181997}"
#define MyAppName "华成机器人仿真平台"
#define MyAppNameZh "华成机器人仿真平台"
#define MyAppVersion "1.2.9"
#define MyAppPublisher "szhc"
#define MyAppURL "www.hc-system.com"
; 打包文件
#define MyAppExeName "RobotSimulation.exe"
;#define MyExcludes  "\db,\temp,\Log,\wwwroot\school,\wwwroot\tongping,app.config,app_service.xml,app_service_test.xml"
#define MyAppPath ".."
; 打包生成位置
#define OutputPath "..\bin"
; license文件
#define LicenseFilePath ".\setup_resources\license.txt"
; 美化资源文件
#define ResourcesPath ".\setup_resources\*"

[setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
;LicenseFile={#LicenseFilePath}
OutputDir={#OutputPath}
OutputBaseFilename=华成机器人仿真平台安装包 {#MyAppVersion}
SetupIconFile={#MyAppPath}\Resources\logo.ico
Compression=lzma
SolidCompression=yes
PrivilegesRequired=admin
Uninstallable=yes
UninstallDisplayName={#MyAppNameZh}
UninstallDisplayIcon={uninstallexe},0
DefaultGroupName={#MyAppNameZh}
Versioninfodescription={#MyAppName} 安装程序
versioninfocopyright=Copyright(c) 2024
VersionInfoProductName={#MyAppName}
DisableReadyPage=yes
DisableProgramGroupPage=yes
DirExistsWarning=no
DisableDirPage=yes

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkedonce

[Files]
;Source: {#ResourcesPath}; DestDir: {tmp}; Flags: dontcopy solidbreak ; Attribs: hidden system
Source: "{#MyAppPath}\output\bin_release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyAppPath}\output\bin_release\bearer\*"; DestDir: "{app}\bearer"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyAppPath}\output\bin_release\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyAppPath}\output\bin_release\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyAppPath}\output\bin_release\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyAppPath}\output\bin_release\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyAppPath}\output\bin_release\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyAppPath}\output\bin_release\default_robot\*"; DestDir: "{app}\default_robot"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyAppPath}\output\bin_release\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs createallsubdirs   
Source: "{#MyAppPath}\output\bin_release\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyAppPath}\output\bin_release\CHANGELOG"; DestDir: "{app}"; Flags: ignoreversion
;Source: {#SourceFolder};Excludes: "{#MyExcludes}";DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Messages]
SetupAppTitle={#MyAppName}{#MyAppVersion} 安装向导
SetupWindowTitle={#MyAppName}{#MyAppVersion} 安装向导

[Icons]
Name: "{commondesktop}\{#MyAppNameZh}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{group}\{#MyAppNameZh}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\卸载{#MyAppNameZh}"; Filename: "{uninstallexe}"

[Run]
; 设置安装后启动，如果想让程序在后台启动，可以给Flags设置一个 runhidden 参数
Filename: "{app}\{#MyAppExeName}"; WorkingDir: {app};Parameters: /q; Flags: nowait postinstall skipifsilent

