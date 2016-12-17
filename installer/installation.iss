; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=EFM Server
AppVersion=2.0.0
AppPublisher=EPEX
AppPublisherURL=
DefaultDirName={pf}\EPEX
DefaultGroupName=EPEX
UninstallDisplayIcon={app}\EFMServer.exe
Compression=lzma2
SolidCompression=yes
OutputDir="."
;LicenseFile=

[Dirs]
Name: "{app}"; Permissions: users-full

[Files]
Source: "platforms\qwindows.dll"; DestDir: "{app}\bin\platforms"
Source: "sqldrivers\qsqlodbc.dll"; DestDir: "{app}\bin\sqldrivers"
Source: "EFMServer.exe"; DestDir: "{app}\bin"
Source: "libgcc_s_dw2-1.dll"; DestDir: "{app}\bin"
Source: "libEGL.dll"; DestDir: "{app}\bin"
Source: "libstdc++-6.dll"; DestDir: "{app}\bin"
Source: "libGLESV2.dll"; DestDir: "{app}\bin"
Source: "libwinpthread-1.dll"; DestDir: "{app}\bin"
Source: "D3Dcompiler_47.dll"; DestDir: "{app}\bin"
Source: "opengl32sw.dll"; DestDir: "{app}\bin"
Source: "Qt5Core.dll"; DestDir: "{app}\bin"
Source: "Qt5Gui.dll"; DestDir: "{app}\bin"
Source: "Qt5Network.dll"; DestDir: "{app}\bin"
Source: "Qt5Widgets.dll"; DestDir: "{app}\bin"
Source: "Qt5Sql.dll"; DestDir: "{app}\bin"
Source: "Qt5Charts.dll"; DestDir: "{app}\bin"
Source: "Qt5Svg.dll"; DestDir: "{app}\bin"
Source: "epex_logo.ico"; DestDir: "{app}\bin"


[Icons]
Name: "{group}\EPEX"; Filename: "{app}\bin\EFMServer.exe"
Name: "{commondesktop}\EPEX"; Filename: "{app}\bin\EFMServer.exe";