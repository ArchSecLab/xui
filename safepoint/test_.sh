#!/bin/bash
for i in {0..39}
do
cd paralel/dir$i/
chmod +x test_parallel$i.sh
./test_parallel$i.sh &
cd ../..
touch $i
done
