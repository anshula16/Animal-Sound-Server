# Animal-Sound-Server
>A client server application using socket programming. It uses the **TCP protocol(IPv4)**.
>The application makes use of the **select system call and an array of descriptors to handle multiple clients**.

## Prerequisites
> C compiler

## How to use?
>* #### Start the server
>   > 1. Compile [server.c](server.c) (gcc -o server server.c)
>   > 2. Run server (./server)
>* #### Run client
>   > 1. Compile [client.c](client.c) (gcc -o client client.c)
>   > 2. Run client (./client)

## Modules
>- #### [server.c](server.c)
>   >- [X] **Creates** socket and **binds** the IP address and port number
>   >- [X] Accepts **multiple client requests**
>   >- [X] Provides **authentication** of clients through id password and **registration** of new clients
>   >- [X] **Return the sound** corresponding to the requested animal
>   >- [X] **Add** new of animal sound pair
>   >- [X] **Update** sound of existing animals

>- #### [client.c](client.c)
>   >- [X] **Creates** socket and **binds** the IP address and port number
>   >- [X] Provides **login/ registration** of client
>   >- [X] **Request** sound of an animal (**command:** animal_name)
>   >- [X] **Add** new animal sound pair  (**command:** animal_name, animal_sound (newline separated))
>   >- [X] **Update** sound of existing animal  (**command:** animal_name, animal_sound (newline separated))
>   >- [X] **Close conncetion with the server** (**command:** BYE)
>   >- [X] **Close the server** (**command:** END)
