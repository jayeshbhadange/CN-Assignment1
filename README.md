# CN-Assignment1
Team Members

Jayesh Bhadange -20110082

Ekansh Somani-20110065

We have used network interface as loopback(lo) and speed =1Mbps
## Part-1
Turn off your internet connection before doing this <br>
Type the following in terminal to create an executable file of capture.C and then run it:<br>
gcc -o a capture.C <br>
sudo ./a <br>

On another terminal, use the following commands to replay the packets from 0.pcap:<br>
sudo tcpreplay -v --mbps=3 -i <network_interface> 2.pcap <br>

<br>

The captured packet info and the soure IP, destination IP, source port and destination port will
all be stored in log.txt.

## Part 2
Turn off your internet connection before doing this <br>
Type the following in terminal to create an executable file of capture.C and then run it:<br>
gcc -o a part2.C <br>
sudo ./a <br>

On another terminal, use the following commands to replay the packets from 0.pcap:<br>
sudo tcpreplay -v --mbps=3 -i <network_interface> 3.pcap <br>

Answers for part2 CTF can be viewed in ctfans.pdf
## Reference
https://www.binarytides.com/packet-sniffer-code-in-c-using-linux-sockets-bsd-part-2/

