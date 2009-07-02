#!/bin/bash
##############

clientuser=$1
basefolder=$2
client=$3
confuser=root
config=$4
datauser=$1
database=$5
loguser=$1
logging=$6

if [ "$basefolder" = "" ]
then 
  basefolder=..
fi

if [ "$client" = "" ]
then
    echo
    echo "set permission from folder client, data and log in actual workspace to user $clientuser"
    echo "let the current group there also writing for some changing from user"
    chown -R $clientuser $basefolder/client
    chmod -R g+w $basefolder/client
    chown -R $clientuser ../data
    chmod -R g+w $basefolder/data
    chown -R $clientuser ../log
    chmod -R g+w $basefolder/log
else
	if test -d $client
	then
	  echo "set permission $clientuser to all client files in $client"
	  chown -R $clientuser $client
	else
	  echo "generate client folder $client with default content and set permission to $clientuser"
	  mkdir -p $client
	  cp -R $basefolder/client/* $client
	  chown -R $clientuser $client
	fi
	
	echo "generate configuration folder $config with newer config files and set permission to $confuser"
	mkdir -p $config
	cp -uR $basefolder/conf/* $config
	chown -R $confuser $config
	chown root:root $config/access.conf
	chmod 600 $config/access.conf
	
	echo ""
	echo "generate database folder $database and set permission to $datauser"
	mkdir -p $database
	chown -R $datauser $database
	
	echo "generate logging folder $logging and set permission to $loguser"
	mkdir -p $logging
	chown -R $loguser $logging
	echo ""
	
fi