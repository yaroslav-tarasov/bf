#/bin/bash

SINTEZ_HOME=/sintez/sintez


.  $SINTEZ_HOME/.uvd_security
export DISPLAY=:0
case  $1 in

start)


$UVD_SECURITY/bin/bf-service -s  &


;;

stop)

$UVD_SECURITY/bin/bf-service -t  &

;;

*)

echo  "Usage $0 <start|stop>"


;;

esac


