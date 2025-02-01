#!/bin/bash
for i in {0..95..5}
do
    cd paralel/dir$i/
	chmod +x test_parallel$i.sh
	./test_parallel$i.sh &
	cd ../..
	touch $i
done
