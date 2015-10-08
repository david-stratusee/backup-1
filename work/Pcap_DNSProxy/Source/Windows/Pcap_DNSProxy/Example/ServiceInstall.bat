:: Pcap_DNSProxy install service batch
:: A local DNS server base on WinPcap and LibPcap.
:: 
:: Author: Hugo Chan, Chengr28
:: 

@echo off

:: Permission check
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (set SystemPath = %SystemRoot%\SysWOW64) else (set SystemPath = %SystemRoot%\system32)
::rd "%SystemPath%\Test_Permissions" > nul 2 > nul
::md "%SystemPath%\Test_Permissions" 2 > nul || (echo Require Administrator Permission. && pause > nul && Exit)
::rd "%SystemPath%\Test_Permissions" > nul 2 > nul
del /f /q %SystemPath%\TestPermission.log
echo "Permission check." >> %SystemPath%\TestPermission.log
if not exist %SystemPath%\TestPermission.log (echo Require Administrator Permission. && pause > nul && Exit)
del /f /q %SystemPath%\TestPermission.log

:: Files check
cd /d %~dp0
cls
if not exist Fciv.exe goto Warning
if not exist Pcap_DNSProxy.exe goto Warning
:: if not exist KeyPairGenerator.exe goto Warning
if not exist Pcap_DNSProxy_x86.exe goto Warning
:: if not exist KeyPairGenerator_x86.exe goto Warning

:Hash-A
Fciv -sha1 Pcap_DNSProxy.exe |findstr /I 62E818167427E4E6DA33EF83B3D458C0611089F6 > NUL
goto HASH-%ERRORLEVEL%
:HASH-0
goto HASH-B
:HASH-1
goto Warning

:Hash-B
Fciv -sha1 Pcap_DNSProxy_x86.exe |findstr /I C85F71A58CAFC0505E76CB160B12DE491611B8D3 > NUL
goto HASH-%ERRORLEVEL%
:HASH-0
goto Type
:HASH-1
goto Warning

:Warning
@echo.
@echo The file(s) may be damaged or corrupt!
@echo Please download all files again, also you can skip this check.
:: Choice.exe cannot be run in Windows XP/2003.
:: choice /M "Are you sure you want to continue start service"
:: if errorlevel 2 exit
:: if errorlevel 1 echo.
set /p UserChoice="Are you sure you want to continue start service? [Y/N]"
if /i "%UserChoice%" == "Y" (goto Type) else exit

:: Architecture check and main process
:Type
sc stop PcapDNSProxyService
sc delete PcapDNSProxyService
if "%PROCESSOR_ARCHITECTURE%%PROCESSOR_ARCHITEW6432%" == "x86" (goto X86) else goto X64

:X86
sc create PcapDNSProxyService binPath= "%~dp0Pcap_DNSProxy_x86.exe" start= auto
reg add HKLM\SYSTEM\CurrentControlSet\Services\PcapDNSProxyService\Parameters /v Application /d "%~dp0Pcap_DNSProxy_x86.exe" /f
reg add HKLM\SYSTEM\CurrentControlSet\Services\PcapDNSProxyService\Parameters /v AppDirectory /d "%~dp0" /f
Pcap_DNSProxy_x86 --FirstStart
goto Exit

:X64
sc create PcapDNSProxyService binPath= "%~dp0Pcap_DNSProxy.exe" start= auto
reg add HKLM\SYSTEM\CurrentControlSet\Services\PcapDNSProxyService\Parameters /v Application /d "%~dp0Pcap_DNSProxy.exe" /f
reg add HKLM\SYSTEM\CurrentControlSet\Services\PcapDNSProxyService\Parameters /v AppDirectory /d "%~dp0" /f
Pcap_DNSProxy --FirstStart
goto Exit

:Exit
sc description PcapDNSProxyService "A local DNS server base on WinPcap and LibPcap."
sc start PcapDNSProxyService
ipconfig /flushdns
@echo.
@echo Done. Please confirm the PcapDNSProxyService service had been installed.
@echo.
@pause
