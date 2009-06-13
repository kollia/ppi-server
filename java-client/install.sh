#!/bin/bash
##############

if [ -z $1 ]
then
  package=x86
else
  package=x86_64
fi

echo ""
if [ $package = x86 ]
then
  echo "(if you need an x86_64 bit package break with STR + C and type > ./install.sh 64)"
fi
echo "install binary as $package under:"
read -p "[/usr/local]: " prefix

if [ -z $prefix ]
then
  prefix=/usr/local
fi

prefix="$prefix`echo /bin/ppi-server-java-client`"

echo ""
echo "install binary under: $prefix as $package package"

mkdir -p $prefix/lib
cp client.ini $prefix
cp language.txt $prefix
cp ppi-server-java-client.jar $prefix
cp lib/swt.native.jar.gtk.linux.$package $prefix/lib/swt.native.jar
cp -R lib $prefix
cp -R icons $prefix
rm $prefix/lib/swt.native.jar.*

# create start script for ppi-client
echo "#!/bin/bash" > $prefix/start.sh
echo "# starting LGPL java-client" >> $prefix/start.sh
echo "##############################" >> $prefix/start.sh
echo  >> $prefix/start.sh
echo "olddir=\`pwd\`" >> $prefix/start.sh
echo "cd $prefix" >> $prefix/start.sh
echo "java -jar $prefix/ppi-server-java-client.jar \$*" >> $prefix/start.sh
echo "cd \$olddir" >> $prefix/start.sh
chmod +x $prefix/start.sh
ln -s $prefix/start.sh /bin/ppi-client






