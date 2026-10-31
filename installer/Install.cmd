@echo off
rem Opens the GL adapter installer (asks for administrator rights).
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Installer.ps1"
