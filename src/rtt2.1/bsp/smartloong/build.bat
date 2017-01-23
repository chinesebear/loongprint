@echo off
setlocal EnableDelayedExpansion
echo 智龙build
echo 文件所在路径：!cd!
cd !cd!
scons -j4
pause
