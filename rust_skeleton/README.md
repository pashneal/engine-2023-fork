# Dependencies 

There are a few dependencies inherited from the C++ portion of the code, 
you can install them by doing the linux equivalent of 

```bash
apt install libfmt-dev libboost-all-dev cmake clang
```

For arch-based:

```bash
pacman -S fmt boost cmake clang
```

# Installation 

To install, you'll need to build the rust skeleton shared library
this can be achieved by doing the following  

```bash
./rust_build.sh && ./build.sh
```

# Scrimmage Server

When running on the scrimmage server only run the following

```bash
./rust_build.sh
```
And make sure that the `librust_skeleton.so` is correct for the scrimmage architecture

# Troubleshooting

If you see the following error
```
make[2]: *** No rule to make target '../librust_skeleton.so', needed by 'pokerbot'.  Stop.
```

Be sure to run `./rust_build.sh` first and resolve any errors in the rust compilation first.
