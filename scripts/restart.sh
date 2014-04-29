

service bfservice stop
cd /home/sintez/bf/bf-ko
./do_test.sh && service bfservice start 
cd /home/sintez/bin
./bfctl --new INPUT --srcport 53 --proto UDP





 