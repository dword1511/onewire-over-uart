:: Requires Windows Vista or later.
@echo off > nul
title One-wire Temperature

chgport
set /p "port=Select COM port: COM"
set port=COM%port%
echo Selected port "%port%"

:loop

cls

digitemp %port%

timeout /t 1 /nobreak > nul
goto loop
