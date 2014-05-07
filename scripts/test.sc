send(IP(dst="192.9.206.14")/fuzz(UDP(dport=[18000,22,9000])/NTP(version=4)),loop=1)

send(IP(dst="192.9.206.189")/fuzz(UDP(dport=[18000,22,9000]))/"TEST",loop=1)
