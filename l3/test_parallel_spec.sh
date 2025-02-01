python3 x86-ubuntu-run-with-kvm-uipi-template-processor-spec.py --int ${int_type} --interval ${interrupt_interval} --t ${is_hard_timer} --test ${test_name}
rm m5out/stats.txt
while [ ! -s m5out/stats.txt ]
do
./g x86-ubuntu-run-with-kvm-uipi-spec.py
done
cp m5out/stats.txt ../../stats/statOverhead${int_type_full}${interrupt_interval}_hard${is_hard_timer}_${test_name}.txt
cp m5out/board.pc.com_1.device ../../boards/boardOverhead${int_type_full}${interrupt_interval}_hard${is_hard_timer}_${test_name}
touch ../../doneOverhead${int_type_full}${interrupt_interval}_hard${is_hard_timer}_${test_name}
