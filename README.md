# ghost-dns
Reroutes all DNS traffic by (ab)using an `LD_PRELOAD` hack

# Building:
```
$ make
```
# How To Use
```
$ ./bin/run <program>
```
# Example usage
```
$ echo 'google.com = 127.0.0.1' > /etc/ghost.conf
$ ./bin/run wget 'https://google.com/'
```

# Configuration

Currently, the configuration file is located at /etc/ghost.conf

# Translations
Edit /etc/ghost.conf. To setup a translation use the following example:
```
some-website.com = 127.0.0.1 
```
The above example will replace all DNS calls to resolve 'some-website.com' to 127.0.0.1

# Debug output
Lots of debug output if you're using large files. You can comment out the GDNS_DEBUG function *body*. Commenting out the define macro itself will likely result in build-time errors.

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
- Regular expression matching of hosts
- Whitelisting. Any hosts that aren't in /etc/ghost.conf will be redirected to a black hole
- Blacklisting.

# Like to have 
- Allow user to specify a library and function to load. This function will be passed the host name and it shall return the translated IP
