
for i in {0..39}
do
echo "800000" | sudo tee /sys/devices/system/cpu/cpu$i/cpufreq/scaling_min_freq
echo "3900000" | sudo tee /sys/devices/system/cpu/cpu$i/cpufreq/scaling_max_freq
done
