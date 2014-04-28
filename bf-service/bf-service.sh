#/bin/bash

SINTEZ_HOME=/sintez/sintez
UVD_SECURITY=$SINTEZ_HOME/security

bfkorunning() {
  lsmod | grep -q "bf_filter"
}

start() {

bfkorunning || {
    echo "Bf-filter not loaded"
    exit 1
}
$UVD_SECURITY/bin/bf-service -s  &  > /dev/null

}

stop() {

 $UVD_SECURITY/bin/bf-service -t  & > /dev/null

}

restart() {
      stop
      start
}

force_reload(){
      restart
}



#.  $SINTEZ_HOME/.uvd_security
export DISPLAY=:0

case  $1 in

start)
  start
  ;;
stop)
  stop
  ;;
restart)
  restart
  ;;
setup)
  ;;
cleanup)
  ;;

*)

echo  "Usage $0 <start|stop>"

;;

esac


