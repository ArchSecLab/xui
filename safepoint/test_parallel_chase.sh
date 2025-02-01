python3 x86-ubuntu-run-with-kvm-uipi-template-processor-chase.py --int ${int_type} --td ${time_delta}  --si ${size_in}
rm m5out/stats.txt
while [ ! -s m5out/stats.txt ]
do
./g x86-ubuntu-run-with-kvm-uipi-chase.py
done
cp m5out/board.pc.com_1.device ../../boards/boardchase${int_type_full}_${time_delta}_${size_in}
cp m5out/stats.txt ../../stats/statschase${int_type_full}_${time_delta}_${size_in}
touch ../../donechase${int_type_full}_${time_delta}_${size_in}