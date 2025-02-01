
#python3 configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor.py --int Flush --tc ch 
#./build/X86/gem5.debug ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
#cp m5out/stats.txt statFlushHit.txt


#python3 configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor.py --int Drain --tc ch 
#./build/X86/gem5.debug ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
#cp m5out/stats.txt statDrainHit.txt
for i in {0..700..10}
do
rm m5out/stats.txt
python3 configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor.py --int Flush --tc ch --si 0 --td $i
while [ ! -s m5out/stats.txt ]
do
timeout 1s ./build/X86/gem5.fast  ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
timeout 1s ./build/X86/gem5.fast  ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
timeout 1s ./build/X86/gem5.fast  ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
timeout 1m ./build/X86/gem5.fast ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
done
cp m5out/stats.txt stats/statFlushHitPeriod$i.txt
rm m5out/stats.txt
python3 configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor.py --int Flush --tc ch --si 31 --td $i
while [ ! -s m5out/stats.txt ]
do
timeout 1s ./build/X86/gem5.fast  ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
timeout 1s ./build/X86/gem5.fast  ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
timeout 1s ./build/X86/gem5.fast  ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
timeout 1m ./build/X86/gem5.fast ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
done
cp m5out/stats.txt stats/statFlushMissPeriod$i.txt

# rm m5out/stats.txt
# python3 configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor.py --int Drain --tc ch --si 0 --td $i
# while [ ! -s m5out/stats.txt ]
# do
# timeout 1m ./build/X86/gem5.debug ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
# done
# cp m5out/stats.txt stats/statDrainHitPeriod$i.txt
# 
# rm m5out/stats.txt
# python3 configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor.py --int Drain --tc ch --si 31 --td $i
# while [ ! -s m5out/stats.txt ]
# do
# timeout 1m ./build/X86/gem5.debug ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
# done
# cp m5out/stats.txt stats/statDrainMissPeriod$i.txt
# 
# rm m5out/stats.txt
# python3 configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor.py --int N --tc nch --si 0 --td $i
# while [ ! -s m5out/stats.txt ]
# do
# timeout 1m ./build/X86/gem5.debug ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
# done
# cp m5out/stats.txt stats/statNoInterruptHitPeriod$i.txt
# 
# rm m5out/stats.txt
# python3 configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor.py --int N --tc nch --si 31 --td $i
# while [ ! -s m5out/stats.txt ]
# do
# timeout 1m ./build/X86/gem5.debug ./configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi.py
# done
# cp m5out/stats.txt stats/statNoInterruptMissPeriod$i.txt
done
