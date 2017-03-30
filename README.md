# ghost-dns
Reroutes all DNS traffic by (ab)using an LD_PRELOAD hack

# Building:
Run the "m" script. I know, I know, it should be a makefile. Whatever.
```
$ ./m
```
# How To Use
```
$ ./run.sh <program>
```
# Configuration

Currently, the configuration file is located at /etc/ghost.conf
To change the configuration file location, define `GHOSTDNS_CONFIG_FILE` when compiling

# Translations
Edit /etc/ghost.conf. To setup a translation use the following example:
```
some-website.com = 127.0.0.1 
```
The above example will replace all DNS calls to resolve 'some-website.com' to 127.0.0.1

# "All" feature
Edit /etc/ghost.conf. To make use of the "All" feature, use the following example:
```
!all = 192.8.33.4
```
This line of code will translate all DNS requests to the IP `192.8.33.4`. 

# "localhost" feature
Edit /etc/ghost.conf. The "localhost" feature is merely a shortcut for `!all = 127.0.0.1`. To use it, put this in your config file:
```
!localhost
```
And that's it. Every request will be redirected to localhost

# Planned features
- Daemon that creates a shared memory object that allows the LD_PRELOAD'd process to access when trying to get dns translations/settings
- Regular expression matching of hosts
- Whitelisting. Any hosts that aren't in /etc/ghost.conf will be redirected to a black hole
- Blacklisting.

# Need to fix
- As of now, the code is a mix between C style memory allocs (linked lists) and C++ style stuff. I want to eventually replace the linked lists with stl containers

# Like to have 
- Allow user to specify a library and function to load. This function will be passed the host name and it shall return the translated IP
