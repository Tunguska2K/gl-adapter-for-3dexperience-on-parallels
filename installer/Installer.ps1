<#
GL adapter installer for 3DEXPERIENCE on Parallels (Windows 11 on ARM).

Copies opengl32.dll (from the folder this script is in) into the 3DEXPERIENCE program
folder ...\win_b64\code\bin, keeping a backup of any opengl32.dll that is already there.

GUI:            double-click Install.cmd
Command line:   powershell -ExecutionPolicy Bypass -File Installer.ps1 -Action install   -Path "<...\win_b64\code\bin>" [-SetWarningVar]
                powershell -ExecutionPolicy Bypass -File Installer.ps1 -Action uninstall -Path "<...\win_b64\code\bin>"
                powershell -ExecutionPolicy Bypass -File Installer.ps1 -Action status    -Path "<...\win_b64\code\bin>"
                (-Action list prints the 3DEXPERIENCE folders found on this PC)
Writing into "C:\Program Files" needs administrator rights; the GUI asks for them itself.
#>
param(
    [ValidateSet('gui', 'install', 'uninstall', 'status', 'list')]
    [string]$Action = 'gui',
    [string]$Path = '',
    [switch]$SetWarningVar,
    [switch]$Force,
    [switch]$SelfTest
)
$ErrorActionPreference = 'Stop'
$Marker = 'GLADAPTER-3DX-MARKER'
$BackupName = 'opengl32.dll.before-gladapter'
$Here = Split-Path -Parent $PSCommandPath
$SourceDll = Join-Path $Here 'opengl32.dll'
$script:LogSink = { param($m) Write-Host $m }
function Say([string]$m) { & $script:LogSink $m }

function Test-Admin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    return (New-Object Security.Principal.WindowsPrincipal($id)).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# folders that contain 3DEXPERIENCE.exe below "Dassault Systemes"
function Find-3dxBin {
    $found = @()
    foreach ($root in @($env:ProgramFiles, ${env:ProgramW6432}) | Select-Object -Unique) {
        if (-not $root) { continue }
        $ds = Join-Path $root 'Dassault Systemes'
        if (-not (Test-Path $ds)) { continue }
        foreach ($rel in Get-ChildItem $ds -Directory -ErrorAction SilentlyContinue) {
            $bin = Join-Path $rel.FullName 'win_b64\code\bin'
            if (Test-Path (Join-Path $bin '3DEXPERIENCE.exe')) { $found += $bin }
        }
    }
    return $found | Select-Object -Unique
}

# returns the marker text ("GLADAPTER-3DX-MARKER version x.y.z") or $null
function Get-AdapterMarker([string]$file) {
    if (-not (Test-Path $file)) { return $null }
    $bytes = [IO.File]::ReadAllBytes($file)
    $text = [Text.Encoding]::ASCII.GetString($bytes)
    $i = $text.IndexOf($Marker)
    if ($i -lt 0) { return $null }
    $end = $text.IndexOf([char]0, $i)
    if ($end -lt 0) { $end = [Math]::Min($text.Length, $i + 64) }
    return $text.Substring($i, $end - $i)
}

function Get-Status([string]$bin) {
    $dll = Join-Path $bin 'opengl32.dll'
    $lines = @()
    if (-not (Test-Path $bin)) { return @("Folder not found: $bin") }
    if (-not (Test-Path (Join-Path $bin '3DEXPERIENCE.exe'))) { $lines += 'Note: 3DEXPERIENCE.exe is not in this folder.' }
    if (Test-Path $dll) {
        $m = Get-AdapterMarker $dll
        if ($m) { $lines += "Installed: GL adapter ($($m.Replace($Marker, '').Trim()))" }
        else { $lines += 'opengl32.dll present, but it is NOT the GL adapter (another OpenGL library, e.g. Mesa).' }
    } else {
        $lines += 'GL adapter not installed (no opengl32.dll; 3DEXPERIENCE uses the Windows/Parallels driver directly).'
    }
    if (Test-Path (Join-Path $bin $BackupName)) { $lines += "Backup of the previous opengl32.dll: $BackupName" }
    $v = [Environment]::GetEnvironmentVariable('CATForceNotCertifiedGraphics', 'User')
    $lines += "CATForceNotCertifiedGraphics (user) = $(if ($v) { $v } else { '<not set>' })"
    return $lines
}

function Test-3dxRunning { return [bool](Get-Process 3DEXPERIENCE -ErrorAction SilentlyContinue) }

function Install-Adapter([string]$bin, [bool]$setVar, [bool]$force) {
    if (-not (Test-Path $SourceDll)) { throw "opengl32.dll not found next to the installer ($SourceDll)." }
    if (-not (Get-AdapterMarker $SourceDll)) { throw "The opengl32.dll next to the installer is not the GL adapter." }
    if (-not (Test-Path $bin)) { throw "Folder not found: $bin" }
    if (-not (Test-Path (Join-Path $bin '3DEXPERIENCE.exe')) -and -not $force) {
        throw "3DEXPERIENCE.exe is not in '$bin'. Pick the ...\win_b64\code\bin folder (or use -Force)."
    }
    if (Test-3dxRunning) { throw 'Close 3DEXPERIENCE first.' }
    $dll = Join-Path $bin 'opengl32.dll'
    if ((Test-Path $dll) -and -not (Get-AdapterMarker $dll)) {
        $bak = Join-Path $bin $BackupName
        if (Test-Path $bak) { $bak = $bak + '.' + (Get-Date -Format 'yyyyMMdd-HHmmss') }
        Move-Item $dll $bak
        Say "Kept the previous opengl32.dll as $(Split-Path $bak -Leaf)"
    }
    Copy-Item $SourceDll $dll -Force
    $h1 = (Get-FileHash $SourceDll -Algorithm SHA256).Hash
    $h2 = (Get-FileHash $dll -Algorithm SHA256).Hash
    if ($h1 -ne $h2) { throw 'Copy check failed (hash mismatch).' }
    Say "Installed GL adapter into $bin"
    Say "SHA256 $($h2.Substring(0, 16))..."
    if ($setVar) {
        [Environment]::SetEnvironmentVariable('CATForceNotCertifiedGraphics', '1', 'User')
        Say 'Set CATForceNotCertifiedGraphics=1 for your user (hides the "driver not certified" warning).'
    }
    Say 'Done. Start 3DEXPERIENCE as usual.'
}

function Uninstall-Adapter([string]$bin) {
    if (-not (Test-Path $bin)) { throw "Folder not found: $bin" }
    if (Test-3dxRunning) { throw 'Close 3DEXPERIENCE first.' }
    $dll = Join-Path $bin 'opengl32.dll'
    $bak = Join-Path $bin $BackupName
    if (Test-Path $dll) {
        if (Get-AdapterMarker $dll) { Remove-Item $dll -Force; Say 'Removed the GL adapter.' }
        else { Say 'The opengl32.dll in this folder is not the GL adapter; left it alone.'; return }
    } else { Say 'No opengl32.dll in this folder.' }
    if (Test-Path $bak) { Move-Item $bak $dll; Say "Restored the previous opengl32.dll from $BackupName." }
    Say 'Note: CATForceNotCertifiedGraphics stays set. Remove it in Settings > System > About > Advanced system settings > Environment Variables if you want.'
}

# ---------------------------------------------------------------- command line
if ($Action -ne 'gui') {
    try {
        switch ($Action) {
            'list' { $c = Find-3dxBin; if ($c) { $c } else { 'No 3DEXPERIENCE installation found under Program Files\Dassault Systemes.' } }
            'status' { if (-not $Path) { $Path = @(Find-3dxBin)[0] }; if (-not $Path) { throw 'Give -Path.' }; Get-Status $Path }
            'install' { if (-not $Path) { throw 'Give -Path "<...\win_b64\code\bin>" (see -Action list).' }; Install-Adapter $Path $SetWarningVar.IsPresent $Force.IsPresent }
            'uninstall' { if (-not $Path) { throw 'Give -Path "<...\win_b64\code\bin>".' }; Uninstall-Adapter $Path }
        }
        exit 0
    } catch { Write-Host "ERROR: $($_.Exception.Message)"; exit 1 }
}

# ---------------------------------------------------------------- GUI
if (-not $SelfTest -and -not (Test-Admin)) {
    # program folders need administrator rights: restart elevated (hidden console)
    try {
        Start-Process powershell.exe -Verb RunAs -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-STA', '-WindowStyle', 'Hidden', '-File', "`"$PSCommandPath`"")
    } catch { Write-Host 'Administrator rights are needed to write into the 3DEXPERIENCE program folder.' }
    exit
}

Add-Type -AssemblyName System.Windows.Forms, System.Drawing
[Windows.Forms.Application]::EnableVisualStyles()

$form = New-Object Windows.Forms.Form
$form.Text = 'GL adapter for 3DEXPERIENCE'
$form.Size = New-Object Drawing.Size(700, 470)
$form.StartPosition = 'CenterScreen'
$form.Font = New-Object Drawing.Font('Segoe UI', 9)

$lbl = New-Object Windows.Forms.Label
$lbl.Text = '3DEXPERIENCE program folder (the ...\win_b64\code\bin folder that contains 3DEXPERIENCE.exe):'
$lbl.Location = New-Object Drawing.Point(12, 12); $lbl.Size = New-Object Drawing.Size(660, 20)
$form.Controls.Add($lbl)

$combo = New-Object Windows.Forms.ComboBox
$combo.Location = New-Object Drawing.Point(12, 36); $combo.Size = New-Object Drawing.Size(560, 24)
foreach ($c in Find-3dxBin) { [void]$combo.Items.Add($c) }
if ($combo.Items.Count -gt 0) { $combo.SelectedIndex = 0 }
$form.Controls.Add($combo)

$browse = New-Object Windows.Forms.Button
$browse.Text = 'Browse...'; $browse.Location = New-Object Drawing.Point(582, 35); $browse.Size = New-Object Drawing.Size(90, 26)
$form.Controls.Add($browse)

$chk = New-Object Windows.Forms.CheckBox
$chk.Text = 'Hide Dassault''s "graphics driver not certified" warning (sets CATForceNotCertifiedGraphics=1 for your user)'
$chk.Location = New-Object Drawing.Point(12, 70); $chk.Size = New-Object Drawing.Size(660, 22); $chk.Checked = $true
$form.Controls.Add($chk)

$btnInstall = New-Object Windows.Forms.Button
$btnInstall.Text = 'Install'; $btnInstall.Location = New-Object Drawing.Point(12, 102); $btnInstall.Size = New-Object Drawing.Size(110, 30)
$form.Controls.Add($btnInstall)
$btnUninstall = New-Object Windows.Forms.Button
$btnUninstall.Text = 'Uninstall'; $btnUninstall.Location = New-Object Drawing.Point(132, 102); $btnUninstall.Size = New-Object Drawing.Size(110, 30)
$form.Controls.Add($btnUninstall)
$btnClose = New-Object Windows.Forms.Button
$btnClose.Text = 'Close'; $btnClose.Location = New-Object Drawing.Point(562, 102); $btnClose.Size = New-Object Drawing.Size(110, 30)
$form.Controls.Add($btnClose)

$box = New-Object Windows.Forms.TextBox
$box.Multiline = $true; $box.ReadOnly = $true; $box.ScrollBars = 'Vertical'
$box.Location = New-Object Drawing.Point(12, 144); $box.Size = New-Object Drawing.Size(660, 270)
$box.Font = New-Object Drawing.Font('Consolas', 9)
$form.Controls.Add($box)

$script:LogSink = { param($m) $box.AppendText($m + "`r`n") }
function Show-Status {
    $box.Clear()
    $p = $combo.Text.Trim()
    if (-not $p) { Say 'No 3DEXPERIENCE installation found automatically. Use Browse... to pick the ...\win_b64\code\bin folder.'; return }
    foreach ($l in Get-Status $p) { Say $l }
    Say ''
}

$browse.Add_Click({
    $dlg = New-Object Windows.Forms.FolderBrowserDialog
    $dlg.Description = 'Pick the 3DEXPERIENCE ...\win_b64\code\bin folder'
    if ($combo.Text -and (Test-Path $combo.Text)) { $dlg.SelectedPath = $combo.Text }
    if ($dlg.ShowDialog($form) -eq 'OK') { $combo.Text = $dlg.SelectedPath; Show-Status }
})
$combo.Add_SelectedIndexChanged({ Show-Status })
$combo.Add_Leave({ Show-Status })
$btnInstall.Add_Click({
    try {
        $p = $combo.Text.Trim()
        $force = $false
        if ($p -and (Test-Path $p) -and -not (Test-Path (Join-Path $p '3DEXPERIENCE.exe'))) {
            $r = [Windows.Forms.MessageBox]::Show($form, "3DEXPERIENCE.exe is not in this folder.`r`nInstall here anyway?", 'GL adapter', 'YesNo', 'Warning')
            if ($r -ne 'Yes') { return }
            $force = $true
        }
        Install-Adapter $p $chk.Checked $force
    } catch { Say "ERROR: $($_.Exception.Message)" }
})
$btnUninstall.Add_Click({ try { Uninstall-Adapter $combo.Text.Trim() } catch { Say "ERROR: $($_.Exception.Message)" } })
$btnClose.Add_Click({ $form.Close() })
$form.Add_Shown({ Show-Status })

if ($SelfTest) { Show-Status; Write-Host ('selftest: form built, ' + $combo.Items.Count + ' folder(s) found; status text:'); Write-Host $box.Text; $form.Dispose(); exit 0 }
[void]$form.ShowDialog()
