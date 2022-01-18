To compile:
Run 'make all'

To run the code:
./server port_number &
./client ip_address port_number
For example:
./server 8080 &
./client "192.168.0.177" 8080
Note : if binding fails try different port numbers

Testing:
Just type 'help' at any time to know more about the various commands available.
Main prompt commands:

Listusers : List of users separated by spaces
Adduser : Add user to server
This command has been modified to include password. Every user needs to have an associated password.
SetUser : Select user
The user's password needs to be entered to set the user correctly.
Quit : Close connection with server
help : help

Subprompt commands:

Read : Read current message
Delete : Delete current message
Send <receiverid1> <receiverid2> ... <receiveridn> : Send message to specified users
Forward <receiverid1> <receiverid2> ... <receiveridn> : Forward message to specified users
Done : Done with current user
help : help