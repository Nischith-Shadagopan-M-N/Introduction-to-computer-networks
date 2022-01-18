- Run "make GBN" and "make SR" respectively to generate the executables for Go-Back-N and Selective Repeat. "make clean" removes the executables. 
- The sender and receiver for both protocols are setup only for local communication and not for communication between machines. 
- The specific command line arguments for each protocol can be printed by running "./SenderGBN" or "./ReceiverGBN" or "./SenderSR" or "./ReceiverSR" respectively. 

Usage for ./SenderGBN : 
Usage :
./SenderGBN ­-d -s <receiver_IP> -p <receiver_port_num> -l <packet_length> -r <packet_gen_rate> -n <max_packets> -w <window_size> -b <max_buffer_size>

Usage for ./ReceiverGBN : 
Usage :
./ReceiverGBN ­-d -p <receiver_port_num> -n <max_packets> -w <sender_window_size> -e <packet_error_rate>

Usage for ./SenderSR :
Usage : ./SenderSR ­-d -s <receiver_IP> -p <receiver_port_num> -n <sequence_number_bits> -L <max_packet_length> -R <packet_gen_rate> -N <max_packets> -W <window_size> -B <max_buffer_size>


Usage for ./ReceiverSR: 
Usage : ./ReceiverSR ­-d -p <receiver_port_num> -N <max_packets> -n <sequence_number_bits> -W <sender_window_size> -B <max_buffer_size> -e <packet_error_rate>