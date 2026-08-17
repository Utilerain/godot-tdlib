# .\gen\godot.exe - path to godot executable
Start-Process -FilePath ".\gen\godot.exe" -ArgumentList "--path ./example --doctool ./ --gdextension-docs" -Wait -NoNewWindow

if (Test-Path "./example/doc_classes") {
    Remove-Item -Force -Recurse "./example/doc_classes"
}