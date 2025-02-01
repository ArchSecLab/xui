# Uninstall MSR driver
rm -rf /dev/uittmon
rmmod --force uittmon
# modprobe -r MSRdrv
