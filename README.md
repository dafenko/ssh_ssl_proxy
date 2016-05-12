# ssh_ssl_proxy
Transparent SSH and SSL proxy.
1. Listens on defined port (443) 
2. It recognizes incoming connection ssl/tls or ssh
3. Depending on incoming connection proxy forwards to https (443) or ssh (22)

-----------------------------------------
a) Installation
via dpkg

b) Configuration
ssh_ssl_proxy has one configuration file /etc/ssh_ssl_proxy

c) Start stop restart
/etc/init.d/ssh_ssl_proxy start
/etc/init.d/ssh_ssl_proxy stop
/etc/init.d/ssh_ssl_proxy restart

d) Logs into /var/log/ssh_ssl_proxy.log





