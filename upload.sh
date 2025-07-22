make

(
    cd build_switch
    make -j 8 OpenShock.nro
    
    ftp -n <<EOF
open 10.100.100.102
user anonymous
cd /switch/
put OpenShock.nro
EOF

)