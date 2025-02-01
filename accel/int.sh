for i in {0..31}
do
cat stats/statDrainSize$i.txt | grep "board.processor.switch1.core.numUserInterrupts " 
#| awk '{print $2}'
done
