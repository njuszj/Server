#!/bin/expect
set timeout 30
spawn scp /home/onda/chat/server ubuntu@129.211.90.130:~/chat
expect "password:"
send "19994235Sszj\n"
interact
exit

