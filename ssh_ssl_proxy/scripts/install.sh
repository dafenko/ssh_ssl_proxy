#!/bin/bash

function install {
	BASE_DIR=$1
	echo $BASE_DIR
	cp -f $BASE_DIR/ssh_ssl_proxy /usr/sbin/ || exit 21
	cp -f $BASE_DIR/../scripts/init.d/ssh_ssl_proxy /etc/init.d/ || exit 22
	cp -f $BASE_DIR/../ssh_ssl_proxy.conf /etc/ || exit 23
}

function uninstall {
	rm -f /usr/sbin/ssh_ssl_proxy
	rm -f /etc/ssh_ssl_proxy.conf
	rm -f /etc/init.d/ssh_ssl_proxy
}

function make_deb {
	PROJECT=sshsslproxy
	VERSION=1.0-0
	BASE_DIR=$1
	#clean
	rm -rf $BASE_DIR/"$PROJECT"_"$VERSION"
	#create filesystem
	mkdir $BASE_DIR/"$PROJECT"_"$VERSION"
	mkdir -p $BASE_DIR/"$PROJECT"_"$VERSION"/usr/sbin
	mkdir $BASE_DIR/"$PROJECT"_"$VERSION"/etc
	mkdir $BASE_DIR/"$PROJECT"_"$VERSION"/etc/init.d
	#copy files	
	cp -f $BASE_DIR/ssh_ssl_proxy $BASE_DIR/"$PROJECT"_"$VERSION"/usr/sbin/ || return 2
	cp -f $BASE_DIR/../scripts/init.d/ssh_ssl_proxy $BASE_DIR/"$PROJECT"_"$VERSION"/etc/init.d/
	cp -f $BASE_DIR/../ssh_ssl_proxy.conf $BASE_DIR/"$PROJECT"_"$VERSION"/etc/
	#create DEBIAN controll
	mkdir $BASE_DIR/"$PROJECT"_"$VERSION"/DEBIAN
	cp -f $BASE_DIR/../scripts/DEBIAN/postinst $BASE_DIR/"$PROJECT"_"$VERSION"/DEBIAN/
	cp -f $BASE_DIR/../scripts/DEBIAN/postrm $BASE_DIR/"$PROJECT"_"$VERSION"/DEBIAN/ 
	cat > $BASE_DIR/"$PROJECT"_"$VERSION"/DEBIAN/control << _DESC
Package: $PROJECT
Version: $VERSION
Section: base
Priority: optional
Architecture: amd64
Depends: libboost-system1.54.0
Maintainer: Daniel Ferenci <dafe@dafe.net>
Description: SSH SSL Proxy
 Transparent proxy for SSH and SSL connections.
 If the connections is ssh then its forwarded to forward_port_ssh. 
 If the connections is ssl then its forwarded to forward_port_ssl.
_DESC
	dpkg-deb --build "$PROJECT"_"$VERSION"
}

# first argument must be install or uninstall
case $1 in
install)
  install $2
  ;;
uninstall)
  uninstall
  ;;
make_deb)
  make_deb $2
  ;;
*)
  echo "Not supported argument"
  exit 1
  ;;
esac
