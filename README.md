# ghost-dns
Reroutes all DNS traffic by (ab)using an LD_PRELOAD hack

# Building:
Run the "m" script. I know, I know, it should be a makefile. Whatever.
$ ./m

# How To Use

$ ./run.sh <program>

# Examples

$ ./run.sh hhvm get.php     # Reroutes all DNS calls to resolve localhost

# Coming soon
Custom routing based on /etc/ghost.conf
