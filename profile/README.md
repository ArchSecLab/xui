Instructions to run reverse engineering experiments (Figure 3)

## Reverse engineering experiment
This experiment characterizes the latencies of different parts of the interrupt notification, delivery and handling process.
The experiment is divided into three parts, the first part figures out how much time is taken for an apic message to be sent, by short-circuiting senduipi, the second part finds much time is taken to flush the pipeline and modify notification buffers, for this part before we send an interrupt we buffer a higher priority interrupt, and we execute some instructions and check if the receiving core consumed our value, if it has consumed the higher priority interrupt that means the latency value we take is higher, otherwise the sender was faster, we balance these values to estimate flushing cost. We than check how much time it takes for the notifications to be cleared. Because we scramble with the interrupts, it is not possible to keep track of when interrupt delivery and handler code finish; to solve this we run the same test without sending an extra interrupt and monitor events that way.

## Build 
Go to `user_code` directory
```
cd user_code
```

Compile tests:
```
make
```

## Run experiments
There is a frequency setting we need to do
```
./premon.sh
```
To see first part of results do:
```
./send
```
To see the second part of results do:
```
./delivery
```

To see the third part of results do:
```
./return
```

Reset the frequencies back
```
./postmon.sh
```
## How to interpret the results ( you want to follow along with Figure 3 at this point):

Send without receive: 312.578234 -> Before APIC
Send with receive: 398.029486 -> With APIC

FLUSH cycle count: 424.016950 --> FLUSH CYCLES

-- Notification Updates start
Status change cycle count: 31.653410
Posted interrupt clear cycle count: 29.695480
-- PUIR clean 60~ cycles

Delivery first stack push cycle count: 141.568915 -> RSP
Delivery vector push change cycle count: 29.265090 -> Vector
Handler push cycle count: 30.394710 -> handler start operation
uiret push cycle count: 9.211040 -> handler done.
