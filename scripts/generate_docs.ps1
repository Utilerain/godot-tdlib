# .\gen\godot.exe - path to godot executable
$RootDir = Resolve-Path "$PSScriptRoot\.."

Start-Process -FilePath ".\gen\godot.exe" `
    -ArgumentList "--path ./example --doctool ""$RootDir"" --gdextension-docs" `
    -Wait `
    -NoNewWindow