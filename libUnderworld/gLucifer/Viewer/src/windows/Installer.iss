; Installer script for LavaVu

[Setup]
AppName=LavaVu
AppVersion=0.9
DefaultDirName={localappdata}\LavaVu
DefaultGroupName=LavaVu
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=none
ChangesAssociations=yes
OutputBaseFilename=LavaVuSetup
OutputDir=.
SetupIconFile="LavaVu.ico"

[Files]
Source: "LavaVu.exe"; DestDir: "{app}"
Source: "SDL.dll"; DestDir: "{app}"
Source: "sqlite3.dll"; DestDir: "{app}"
Source: "libpng12.dll"; DestDir: "{app}"
Source: "zlib1.dll"; DestDir: "{app}"
Source: "pthreadVCE2.dll"; DestDir: "{app}"
Source: "..\shaders\*.frag"; DestDir: "{app}"
Source: "..\shaders\*.vert"; DestDir: "{app}"
Source: "..\html\*.*"; DestDir: "{app}\html"
Source: "\Windows\System32\msvcrt.dll"; DestDir: "{app}"
Source: "\Windows\System32\msvcp100.dll"; DestDir: "{app}"
Source: "\Windows\System32\msvcr100.dll"; DestDir: "{app}"

[Registry]
Root: HKCU; Subkey: "SOFTWARE\Classes\.gldb"; ValueType: string; ValueName: ""; ValueData: "GraphicsDatabase"; Flags: uninsdeletevalue
Root: HKCU; Subkey: "SOFTWARE\Classes\GraphicsDatabase"; ValueType: string; ValueName: ""; ValueData: "GraphicsDatabase"; Flags: uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\GraphicsDatabase\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\LavaVu.exe,0"
Root: HKCU; Subkey: "SOFTWARE\Classes\GraphicsDatabase\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\LavaVu.exe"" ""%1"""

[Icons]
Name: "{group}\LavaVu"; Filename: "{app}\LavaVu.exe"
Name: "{group}\Uninstall LavaVu"; Filename: "{uninstallexe}"
Name: "{commondesktop}\LavaVu"; Filename: "{app}\LavaVu.exe"

[UninstallDelete]
Type: dirifempty; Name: "{localappdata}\LavaVu"
