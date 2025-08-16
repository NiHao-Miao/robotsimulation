; ��ִ��Ŀ¼Ϊ setup.iss ���ڵ�Ŀ¼����ע��ƴ�����Ŀ¼
#define MyAppId "{{8FDFC065-7DB0-4935-B9A8-756157181997}"
#define MyAppName "���ɻ����˷���ƽ̨"
#define MyAppNameZh "���ɻ����˷���ƽ̨"
#define MyAppVersion "1.2.9"
#define MyAppPublisher "szhc"
#define MyAppURL "www.hc-system.com"
; ����ļ�
#define MyAppExeName "RobotSimulation.exe"
;#define MyExcludes  "\db,\temp,\Log,\wwwroot\school,\wwwroot\tongping,app.config,app_service.xml,app_service_test.xml"
#define MyAppPath ".."
; �������λ��
#define OutputPath "..\bin"
; license�ļ�
#define LicenseFilePath ".\setup_resources\license.txt"
; ������Դ�ļ�
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
OutputBaseFilename=���ɻ����˷���ƽ̨��װ�� {#MyAppVersion}
SetupIconFile={#MyAppPath}\Resources\logo.ico
Compression=lzma
SolidCompression=yes
PrivilegesRequired=admin
Uninstallable=yes
UninstallDisplayName={#MyAppNameZh}
UninstallDisplayIcon={uninstallexe},0
DefaultGroupName={#MyAppNameZh}
Versioninfodescription={#MyAppName} ��װ����
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
SetupAppTitle={#MyAppName}{#MyAppVersion} ��װ��
SetupWindowTitle={#MyAppName}{#MyAppVersion} ��װ��

[Icons]
Name: "{commondesktop}\{#MyAppNameZh}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{group}\{#MyAppNameZh}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\ж��{#MyAppNameZh}"; Filename: "{uninstallexe}"

[Run]
; ���ð�װ��������������ó����ں�̨���������Ը�Flags����һ�� runhidden ����
Filename: "{app}\{#MyAppExeName}"; WorkingDir: {app};Parameters: /q; Flags: nowait postinstall skipifsilent

