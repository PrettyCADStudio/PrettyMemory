@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0comment-filter.ps1" "%~1"
