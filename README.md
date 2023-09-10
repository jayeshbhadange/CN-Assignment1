# CN-Assignment1
## Part-1
Turn off your internet connection before doing this <br>
Type the following in terminal to create an executable file of capture.C and then run it:<br>
gcc -o a capture.C <br>
sudo ./a <br>

On another terminal, use the following commands to replay the packets from 0.pcap:<br>
sudo tcpreplay -v --mbps=3 -i <network_interface> 0.pcap <br>
