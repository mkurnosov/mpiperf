#!/bin/sh

cd ..
make dist
scp ./mpiperf-*.tar.gz mkurnosov@jet.cpct.sibsutis.ru:~/mpiperf-tests
