#!/bin/bash
##############

basefolder=$1
clientuser=$2
client=$3
confuser=root
config=$4
datauser=$2
database=$5
loguser=$2
logging=$6


#if test -x ../../bin/ppi-server
#then

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
	
	echo "generate database folder $database and set permission to $datauser"
	mkdir -p $database
	chown -R $datauser $database
	
	echo "generate logging folder $logging and set permission to $loguser"
	mkdir -p $logging
	chown -R $loguser $logging
	echo ""
	
	#chown -R nobody:nobody /var/local/ppi-server/client
	#chown -R nobody:nobody /var/local/ppi-server/database
	#chown -R nobody:nobody /var/log/ppi-server
	#chmod 600 /var/log/ppi-server/access.conf
#fi