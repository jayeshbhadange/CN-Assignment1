# CN-Assignment1
## Part-1
Type the following in terminal to create an executable file of capture.C and then run it:
gcc -o a capture.C
sudo ./a

On another terminal, use the following commands to replay the packets from 0.pcap:
sudo tcpreplay -v --mbps=3 -i <network_interface> 0.pcap
