make

ftp -n <<EOF
open 10.100.100.102
user anonymous
cd /switch/
put OpenShock.nro
EOF⏎