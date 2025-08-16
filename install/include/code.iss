[Code]
#include "DllsImport.iss"

var
  BGimg:longint;
  btnShowLicense,MinBtn,CancelBtn,btnBrowser,btnSetup,btnBack:HWND;
  isWelcomePage:boolean;
  pathEdit:tedit;
  labelReadImg,licenseImg,progressbgImg,PBOldProc,labelAutoRunImg:longint;
  checkboxLicense,checkboxAutoRun:HWND;
  licenseRich:TRichEditViewer;  
  DpiScalePctg:integer;
  CurrentDPI:integer;
  
function DpiScale(v:integer):integer;
begin
  Result:=v*DpiScalePctg/1000;
end;

// 安装前强制杀进
// 该函数在安装程序初始化时调用，返回False 将中断安装，True则继续安装.程
function InitializeSetup(): Boolean;
  var ErrorCode: Integer;
  begin
    ShellExec('open','taskkill.exe','/f /im {#MyAppExeName}','',SW_HIDE,ewNoWait,ErrorCode);
    ShellExec('open','tskill.exe',' {#MyAppName}','',SW_HIDE,ewNoWait,ErrorCode);
    result := True;
  end;

// 卸载前强制杀进程
function InitializeUninstall(): Boolean;
  var ErrorCode: Integer;
  begin
    ShellExec('open','taskkill.exe','/f /im {#MyAppExeName}','',SW_HIDE,ewNoWait,ErrorCode);
    ShellExec('open','tskill.exe',' {#MyAppName}','',SW_HIDE,ewNoWait,ErrorCode);
    result := True;
  end;

// 向导调用这个事件函数确定是否在所有页或不在一个特殊页 (用PageID 指定) 显示。如果返回True，将跳过该页；如果你返回False，该页被显示。
// 注意: 这个事件函数不被wpWelcome、wpPreparing 和wpInstalling 页调用，还有安装程序已经确定要跳过的页也不会调用
// wpWelcome, wpLicense, wpPassword, wpInfoBefore, wpUserInfo, wpSelectDir, wpSelectComponents, wpSelectProgramGroup, wpSelectTasks, wpReady, wpPreparing, wpInstalling, wpInfoAfter, wpFinished
function ShouldSkipPage(PageID: Integer): Boolean;
  begin
    if PageID=wpLicense then
      result:=true;
    if PageID=wpInfoBefore then
      result:=true;
    if PageID=wpUserInfo then
      result:=true;
    if PageID=wpSelectDir then
      result:=true;
    if PageID=wpSelectComponents then
      result:=true;
    if PageID=wpSelectProgramGroup then
      result:=true;
    if PageID=wpSelectTasks then
      result:=true;
  end;

// 关闭按钮
procedure CancelBtnOnClick(hBtn:HWND);
  begin
    WizardForm.CancelButton.OnClick(WizardForm);
  end;

// 最小化按钮
procedure MinBtnOnClick(hBtn:HWND);
  begin
    SendMessage(WizardForm.Handle,WM_SYSCOMMAND,61472,0);
  end;

// 浏览文件夹按钮
procedure btnBrowserclick(hBtn:HWND);
  begin
    WizardForm.DirBrowseButton.OnClick(WizardForm);
    pathEdit.text := WizardForm.DirEdit.text;
  end;

// 路径选择器 change
procedure pathEditChange(Sender: TObject);
  begin
    WizardForm.DirEdit.text:=pathEdit.Text ;
  end;

// 立即安装等按钮的操作就是不断地下一步
procedure nextSetpBtnClick(hBtn:HWND);
  begin
    WizardForm.NextButton.OnClick(WizardForm);
  end;

// 安装的进度条
function PBProc(h:hWnd;Msg,wParam,lParam:Longint):Longint;
  var
    pr,i1,i2 : Extended;
    w : integer;
  begin
    Result:=CallWindowProc(PBOldProc,h,Msg,wParam,lParam);
    if (Msg=$402) and (WizardForm.ProgressGauge.Position>WizardForm.ProgressGauge.Min) then
    begin
      i1:=WizardForm.ProgressGauge.Position-WizardForm.ProgressGauge.Min;
      i2:=WizardForm.ProgressGauge.Max-WizardForm.ProgressGauge.Min;
      pr:=i1*100/i2;
      w:=Round(650*pr/100);
      ImgSetPosition(progressbgImg,0,DpiScale(320),DpiScale(w),DpiScale(15));
      ImgSetVisiblePart(progressbgImg,0,0,w,DpiScale(15));
      ImgApplyChanges(WizardForm.Handle);
    end;
  end;

// 鼠标拖动
procedure WizardMouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
  begin
    ReleaseCapture
    SendMessage(WizardForm.Handle, $0112, $F012, 0)
  end;

// 查看 license
procedure btnShowLicenseClick(hBtn:HWND);
  begin
    if isWelcomePage=true then
    begin
	  isWelcomePage:=false
      ImgSetVisibility(licenseImg,true)
      ImgSetVisibility(BGimg,false)
      ImgSetVisibility(labelReadImg,false)
      licenseRich.Height:=DpiScale(295)
      BtnSetVisibility(checkboxLicense,false)
      BtnSetVisibility(btnShowLicense,false)
      BtnSetVisibility(btnSetup,false)
      BtnSetVisibility(btnBrowser,false)
      pathEdit.Hide
      BtnSetVisibility(btnBack,true)
    end else
    begin
      isWelcomePage:=true
      ImgSetVisibility(BGimg,true)
      ImgSetVisibility(licenseImg,false)
      ImgSetVisibility(labelReadImg,true)
      licenseRich.Height:=0
      BtnSetVisibility(checkboxLicense,true)
      BtnSetVisibility(btnShowLicense,true)
      BtnSetVisibility(btnSetup,true)
      BtnSetVisibility(btnBrowser,true)
      pathEdit.show
      BtnSetVisibility(btnBack,false)
    end;
      ImgApplyChanges(WizardForm.Handle)
  end;

// 勾选查看 license
procedure checkboxLicenseClick(hBtn:HWND);
  begin
    if BtnGetChecked(checkboxLicense)=true then
    begin
      BtnSetEnabled(btnSetup,true)
    end else
    begin
      BtnSetEnabled(btnSetup,false)
    end
  end;

// 该过程在开始的时候改变向导或者向导页，不要指望使用InitializeSetup函数实现改变向导页的功能，因为InitializeSetup函数触发时向导窗口并不存在。
procedure InitializeWizard();
   var 
      MainLabel: TLabel;
  begin
  
	CurrentDPI  := WizardForm.Font.PixelsPerInch;
	DpiScalePctg  := 1000* CurrentDPI / 96;
    // 设置欢迎向导页的尺寸大小
    WizardForm.OuterNotebook.hide;
    WizardForm.Bevel.Hide;
    WizardForm.BorderStyle:=bsnone;
    WizardForm.Position:=poScreenCenter;
    // WizardForm.Center;
    WizardForm.Width:=DpiScale(650);
    WizardForm.Height:=DpiScale(450);
    // WizardForm.Color:=clWhite;
    isWelcomePage:=true;
    // WizardForm.InnerNotebook.Hide;
    // wizardform.Color:=TransparentColor;

    // 添加可以拖动窗口的Label
    MainLabel := TLabel.Create(WizardForm);
    MainLabel.Parent := WizardForm;
    MainLabel.AutoSize := False;
    MainLabel.Left := 0;
    MainLabel.Top := 0;
    MainLabel.Width := WizardForm.Width;
    MainLabel.Height := WizardForm.Height;
    MainLabel.Caption := '';
    MainLabel.Transparent := True;
    MainLabel.OnMouseDown := @WizardMouseDown;

    // 加载资源到 tmp 临时目录
    ExtractTemporaryFile('bg_welcome.png');
    ExtractTemporaryFile('bg_installing.png');
    ExtractTemporaryFile('bg_license.png');
    ExtractTemporaryFile('bg_finished.png');
    ExtractTemporaryFile('label_read.png');
    ExtractTemporaryFile('label_license.png');
    ExtractTemporaryFile('license.txt');
    ExtractTemporaryFile('btn_close.png');
    ExtractTemporaryFile('btn_min.png');
    ExtractTemporaryFile('btn_back.png');
    ExtractTemporaryFile('btn_setup.png');
    ExtractTemporaryFile('btn_browser.png');
    ExtractTemporaryFile('checkbox.png');
    ExtractTemporaryFile('btn_complete.png');
    ExtractTemporaryFile('loading.png');
    ExtractTemporaryFile('label_autorun.png');

    // 关闭按钮样式
    CancelBtn:=BtnCreate(WizardForm.Handle,DpiScale(650-10-16),DpiScale(8),DpiScale(16),DpiScale(16),ExpandConstant('{tmp}\btn_close.png'),1,False)
    BtnSetEvent(CancelBtn,BtnClickEventID,WrapBtnCallback(@CancelBtnOnClick,1));
    
    // 最小化按钮样式
    MinBtn:=BtnCreate(WizardForm.Handle,DpiScale(624-10-16),DpiScale(8),DpiScale(16),DpiScale(16),ExpandConstant('{tmp}\btn_min.png'),1,False)
    BtnSetEvent(MinBtn,BtnClickEventID,WrapBtnCallback(@MinBtnOnClick,1));
    
    // 立即安装按钮样式
    btnSetup:=BtnCreate(WizardForm.Handle,DpiScale(195),DpiScale(340),DpiScale(260),DpiScale(44),ExpandConstant('{tmp}\btn_setup.png'),1,False)
    BtnSetEvent(btnSetup,BtnClickEventID,WrapBtnCallback(@nextSetpBtnClick,1));
    
    // 返回按钮样式
    btnBack:=BtnCreate(WizardForm.Handle,DpiScale(285),DpiScale(390),DpiScale(80),DpiScale(32),ExpandConstant('{tmp}\btn_back.png'),1,False)
    BtnSetEvent(btnBack,BtnClickEventID,WrapBtnCallback(@btnShowLicenseClick,1));
    BtnSetVisibility(btnBack,false)
    
    pathEdit:= TEdit.Create(WizardForm);
    with pathEdit do
    begin
      Parent:=WizardForm;
      text:=WizardForm.DirEdit.text;
      Font.Name:='宋体'
      BorderStyle:=bsNone;
      SetBounds(DpiScale(110),DpiScale(272),DpiScale(340),DpiScale(15))
      OnChange:=@pathEditChange;
      TabStop:=false;
    end;

    // 浏览按钮样式
    btnBrowser:=BtnCreate(WizardForm.Handle,DpiScale(470),DpiScale(263),DpiScale(80),DpiScale(32),ExpandConstant('{tmp}\btn_browser.png'),1,False)
    BtnSetEvent(btnBrowser,BtnClickEventID,WrapBtnCallback(@btnBrowserclick,1));

    PBOldProc:=SetWindowLong(WizardForm.ProgressGauge.Handle,-4,PBCallBack(@PBProc,4));

    // 查看 license 的文本区域
    licenseRich:= TRichEditViewer.Create(WizardForm);
    with licenseRich do
    begin
      Parent:=WizardForm;
      ReadOnly:=true;
      SCROLLBARS:=ssVertical;
      font.Name:='宋体';
      font.Size:=12;
      Color:=clWhite;
      BorderStyle:=bsNone;
      SetBounds(DpiScale(40),DpiScale(65),DpiScale(570),0)
      Lines.LoadFromFile(ExpandConstant('{tmp}\license.txt'));
      TabStop:=false;
    end;
    
    ImgApplyChanges(WizardForm.Handle)
  end;

// 在新向导页 (由CurPageID 指定) 显示后调用。
procedure CurPageChanged(CurPageID: Integer);
  begin
    // 默认的流程按钮隐藏掉
    WizardForm.NextButton.Visible:=false;
    WizardForm.CancelButton.Height:=0;
    WizardForm.BackButton.Height:=0;

    if CurPageID=wpWelcome then
      begin
		licenseImg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\bg_license.png'),0,0,DpiScale(650),DpiScale(450),true,true);
		ImgSetVisibility(licenseImg,false)
        BGimg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\bg_welcome.png'),0,0,DpiScale(650),DpiScale(450),true,true);

        // license 提示
        checkboxLicense:=BtnCreate(WizardForm.Handle,DpiScale(220),DpiScale(400),DpiScale(16),DpiScale(16),ExpandConstant('{tmp}\checkbox.png'),1,true)
        BtnSetEvent(checkboxLicense,BtnClickEventID,WrapBtnCallback(@checkboxLicenseClick,1))

        // 默认勾选 阅读并同意
        BtnSetChecked(checkboxLicense,true)
        labelReadImg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\label_read.png'),DpiScale(243),DpiScale(398),DpiScale(39),DpiScale(20),true,true);
        btnShowLicense:=BtnCreate(WizardForm.Handle,DpiScale(282),DpiScale(398),DpiScale(65),DpiScale(20),ExpandConstant('{tmp}\label_license.png'),DpiScale(4),false)
        BtnSetEvent(btnShowLicense,BtnClickEventID,WrapBtnCallback(@btnShowLicenseClick,1))
        
        WizardForm.Width:=DpiScale(650);
        WizardForm.Height:=DpiScale(450);
        WizardForm.Show;
      end;

    if CurPageID = wpInstalling then
      begin
        BtnSetPosition(checkboxLicense,DpiScale(560),DpiScale(421),DpiScale(75),DpiScale(15));
        pathEdit.Hide;
        BtnSetVisibility(btnBrowser,false)
        WizardForm.Height:=DpiScale(450)
        BtnSetVisibility(btnShowLicense,false);
        BtnSetVisibility(checkboxLicense,false);
        BGimg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\bg_installing.png'),0,0,DpiScale(650),DpiScale(450),true,true);
        
        progressbgImg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\loading.png'),0,DpiScale(320),DpiScale(650),0,True,True);

        BtnSetVisibility(btnSetup,false);
      end;

    if CurPageID = wpFinished then
      begin
        ImgSetVisibility(progressbgImg,false)
        btnSetup:=BtnCreate(WizardForm.Handle,DpiScale(195),DpiScale(340),DpiScale(260),DpiScale(44),ExpandConstant('{tmp}\btn_complete.png'),1,False)
        BtnSetEvent(btnSetup,BtnClickEventID,WrapBtnCallback(@nextSetpBtnClick,1));

        BGimg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\bg_finished.png'),0,0,DpiScale(650),DpiScale(450),true,true);

        // 是否自动运行
        checkboxAutoRun:=BtnCreate(WizardForm.Handle,DpiScale(275),DpiScale(400),DpiScale(16),DpiScale(16),ExpandConstant('{tmp}\checkbox.png'),1,true)
        labelAutoRunImg:=ImgLoad(WizardForm.Handle,ExpandConstant('{tmp}\label_autorun.png'),DpiScale(295),DpiScale(398),DpiScale(84),DpiScale(20),true,true);

        // 默认勾选
        BtnSetChecked(checkboxAutoRun,true);

        // 完成页面隐藏关闭和最小化
        BtnSetVisibility(CancelBtn,false);
        BtnSetVisibility(MinBtn,false);
      end;

    ImgApplyChanges(WizardForm.Handle)
  end;
  
// 该过程在安装终止时被调用，注意及时在用户没有安装任何文件之前退出也会被调用。
procedure DeinitializeSetup();
  var RCode: Integer;
  begin
    // 检查是否需要自动运行
    if BtngetChecked(checkboxAutoRun)=true then
    begin
      Exec(ExpandConstant('{app}\{#MyAppExeName}'),'','',SW_SHOW, ewNoWait,RCode);
    end;

    gdipShutdown;
  end;

