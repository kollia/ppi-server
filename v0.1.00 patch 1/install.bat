@echo off

if %1: == : (
	echo .
	echo install [directory]
	echo .     please set any directory as first parameter
	echo .     to the place where you want to install
	echo .     like C:\Programms\ppi-client
	goto end					)

echo .
echo copy client to %1

mkdir %1
mkdir %1\lib
mkdir %1\icons
copy * %1\
copy lib %1\lib\
copy icons %1\icons\
move %1\lib\swt.native.jar.win32.x86 %1\lib\swt.native.jar
del %1\lib\swt.native.jar.gtk*

echo .
echo go into %1 and start client with java -jar ppi-server-java-client.jar
echo or open browser and click on ppi-server-java-client.jar to start

:end