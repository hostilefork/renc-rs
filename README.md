# renc-rs
An experimental Rust binding to [Ren-C](https://github.com/metaeducation/ren-c).

I'm not actively working on this anymore, I'm posting it just in case it might be useful to others.

To run the test (Only supports Linux, because of the prebuilt libr3.so):
#LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/renc-sys/renc/lib/ cargo test -- --test-threads=1
