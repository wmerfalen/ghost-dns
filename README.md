# ghost-dns
Reroutes all DNS traffic by (ab)using an LD_PRELOAD hack

# Building:
Run the "m" script. I know, I know, it should be a makefile. Whatever.
$ ./m

# How To Use

$ ./run.sh <program>

# Examples

$ ./run.sh hhvm get.php     # Reroutes all DNS calls to resolve localhost

# Configuration
Currently, the configuration file is located at /etc/ghost.conf

#Translations
Edit /etc/ghost.conf. To setup a translation use the following example:

some-website.com = 127.0.0.1 

The above example will replace all DNS calls to resolve 'some-website.com' to 127.0.0.1
