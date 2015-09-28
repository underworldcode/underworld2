; Installer script for gLucifer VisViewer

[Setup]
AppName=gLucifer Viewer
AppVersion=0.71
DefaultDirName={localappdata}\gLucifer Viewer
DefaultGroupName=gLucifer Viewer
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=none
ChangesAssociations=yes
OutputBaseFilename=gLuciferServerSetup
OutputDir=.
SetupIconFile="gLuciferViewer.ico"

[Files]
Source: "Release\gLuciferViewer.exe"; DestDir: "{app}"
Source: "SDL.dll"; DestDir: "{app}"
Source: "sqlite3.dll"; DestDir: "{app}"
Source: "libpng12.dll"; DestDir: "{app}"
Source: "zlib1.dll"; DestDir: "{app}"
Source: "pthreadVCE2.dll"; DestDir: "{app}"
;Modified shaders for demo
Source: "*.frag"; DestDir: "{app}"
Source: "*.vert"; DestDir: "{app}"
;Source: "..\shaders\*.frag"; DestDir: "{app}"
;Source: "..\shaders\*.vert"; DestDir: "{app}"
Source: "..\html\*.*"; DestDir: "{app}\html"
Source: "\Windows\System32\msvcrt.dll"; DestDir: "{app}"
Source: "\Windows\System32\msvcp100.dll"; DestDir: "{app}"
Source: "\Windows\System32\msvcr100.dll"; DestDir: "{app}"

[Registry]
Root: HKCU; Subkey: "SOFTWARE\Classes\.gldb"; ValueType: string; ValueName: ""; ValueData: "gLuciferDatabase"; Flags: uninsdeletevalue
Root: HKCU; Subkey: "SOFTWARE\Classes\gLuciferDatabase"; ValueType: string; ValueName: ""; ValueData: "gLucifer Database"; Flags: uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\Classes\gLuciferDatabase\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\gLuciferViewer.exe,0"
Root: HKCU; Subkey: "SOFTWARE\Classes\gLuciferDatabase\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\gLuciferViewer.exe"" ""%1"""

[Icons]
Name: "{group}\gLucifer Viewer"; Filename: "{app}\gLuciferViewer.exe"
Name: "{group}\Uninstall gLucifer Viewer"; Filename: "{uninstallexe}"
Name: "{commondesktop}\gLucifer Viewer"; Filename: "{app}\gLuciferViewer.exe"

[UninstallDelete]
Type: dirifempty; Name: "{localappdata}\gLucifer Viewer"
