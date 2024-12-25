# 349Bot
349Bot is a blazing-fast C++ based Discord music bot.

## Quick Memo for Developing
gpg often fails to sign.

To resolve this issue, I recommend using terminal(WSL terminal/VSCode internal terminal) first to commit, then commit using VSCode embeded Git.

"--pinentry-mode=loopback" and "--no-autostart" works for generating keys, but not for signing.

Some commands from this list will work:

Do this

"echo "hoge" | gpg --clearsign --no-autostart --pinentry-mode=loopback"

and then try signing.

"killall gpg-agent"
"gpgconf --kill gpg-agent"
"gpgconf --launch gpg-agent"
"export GPG_TTY=$(tty)"

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
make --DWITH_OPENSSL
make install
```