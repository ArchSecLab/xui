for i in {0..31}
do
cat stats/statFlushSize$i.txt | grep "board.processor.switch1.core.endToEndSendUipiLatency" | awk '{print $2}'
done
