# 349Bot
349Bot is a blazingly-fast C++ based Discord music bot.

## Quick Memo for Developing
gpg often fails to sign.

This command 
```bash
echo "hoge" | gpg --clearsign --no-autostart --pinentry-mode=loopback
```

## Log level
0: TRACE
1: DEBUG
2: INFO
3: WARN
4: ERROR
5: CRITICAL

## Build
```bash
cmake --build build --config Release
```

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw.cmake -B windows
```

To build libhv, you have to do this:
```bash
./configure
cmake --DWITH_OPENSSL
cmake install
```
