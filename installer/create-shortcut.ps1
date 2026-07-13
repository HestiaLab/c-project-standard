$ws = New-Object -ComObject WScript.Shell
$lnk = $ws.CreateShortcut([Environment]::GetFolderPath('Desktop') + '\CProjectStandard.lnk')
$lnk.TargetPath = $args[0]
$lnk.WorkingDirectory = $args[1]
$lnk.IconLocation = (Join-Path $args[1] "share\c-project-standard\res\cproject.ico")
$lnk.Save()