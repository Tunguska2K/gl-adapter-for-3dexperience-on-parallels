@echo off
rem 3DEXPERIENCE in VMPal: sets the required options. Undo: VMPal-Setup.cmd -Action remove
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0VMPal-Setup.ps1" %*
