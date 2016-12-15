; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=EPEX Server
AppVersion=1.0.0
AppPublisher=EPEX
AppPublisherURL=
DefaultDirName={pf}\EPEX
DefaultGroupName=EPEX
UninstallDisplayIcon={app}\ServerClient.exe
Compression=lzma2
SolidCompression=yes
OutputDir="."
;LicenseFile=

[Dirs]
Name: "{app}"; Permissions: users-full

[Files]
Source: "platforms\qminimal.dll"; DestDir: "{app}\bin\platforms"
Source: "platforms\qoffscreen.dll"; DestDir: "{app}\bin\platforms"
Source: "platforms\qwindows.dll"; DestDir: "{app}\bin\platforms"
Source: "sqldrivers\qsqlodbc.dll"; DestDir: "{app}\bin\sqldrivers"
Source: "ServerClient.exe"; DestDir: "{app}\bin"
Source: "libgcc_s_dw2-1.dll"; DestDir: "{app}\bin"
Source: "libstdc++-6.dll"; DestDir: "{app}\bin"
Source: "libwinpthread-1.dll"; DestDir: "{app}\bin"
Source: "Qt5Core.dll"; DestDir: "{app}\bin"
Source: "Qt5Gui.dll"; DestDir: "{app}\bin"
Source: "Qt5Network.dll"; DestDir: "{app}\bin"
Source: "Qt5Widgets.dll"; DestDir: "{app}\bin"
Source: "Qt5Sql.dll"; DestDir: "{app}\bin"
Source: "epex_logo.ico"; DestDir: "{app}\bin"


[Icons]
Name: "{group}\EPEX"; Filename: "{app}\bin\ServerClient.exe"
Name: "{commondesktop}\EPEX"; Filename: "{app}\bin\ServerClient.exe";