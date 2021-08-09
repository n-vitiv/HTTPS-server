# HTTPS-server
Simple HTTPS-server

Works only at linux. Compile with GNU Make.
Currently server supports only GET method.
You need to use a OpenSSL library to properly compile and run the server.

Config setting:
1. "logs" - 4 mods:
 * 0 - turn off
 * 1 - turn on
 * 2 - turn off only debug logs
 * 3 - turn off debug and warning logs
2. "port" - port number on which server will work
3. "address" - ip address of server
4. "root" - root directory
5. "default" - default page which will be downloaded from the root folder.
