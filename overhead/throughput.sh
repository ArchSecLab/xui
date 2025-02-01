for i in 1 10 100 200 300 400 500 600 700
do
cat "stats/stat12GbpsTrack${i}TIMER.txt"| grep "board.loadgen.LoadGenerator.latency::" | grep "100.00%" | awk '{print $1}' | awk '{gsub(/board.loadgen.LoadGenerator.latency::[0-9]*|/, "");gsub(/-|/, "");  print}' | head -n 1 
done
cat "stats/stat12GbpsTrack0PCI.txt"| grep "board.loadgen.LoadGenerator.latency::" | grep "100.00%" | awk '{print $1}' | awk '{gsub(/board.loadgen.LoadGenerator.latency::[0-9]*|/, "");gsub(/-|/, "");  print}' | head -n 1 
