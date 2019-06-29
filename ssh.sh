#!/bin/expect
set timeout 30
spawn ssh ubuntu@129.211.90.130
expect "password:"
send "19994235Sszj\n"
interact #表示执行完留在远程控制台，否则会直接返回本地控制台 
exit
