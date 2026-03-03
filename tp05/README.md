# TP 5 — `fork`, `exec` and bounded parallelism

| Program | Purpose |
|---|---|
| `bin/simultane` | `du -sh /usr/share & ls -l & ps` |
| `bin/successif` | `du -sh /usr/share ; ls -l ; ps` |
| `bin/miniature` | resize every image of a directory to 10 %, at most 8 `convert` processes |
| `bin/telecharge` | download a list of URLs, at most 4 `wget` processes |
| `bin/make` | compile several `.c` files in parallel and link them if all succeed |

```bash
make
./bin/simultane
./bin/successif
./bin/miniature images thumbs
./bin/telecharge https://example.com/a.txt https://example.com/b.txt downloads
./bin/make samples/success/main.c samples/success/mathutils.c samples/success/message.c demo
./demo
```

`miniature` and `telecharge` keep a pool of at most 8 (respectively 4) active
children.  When the pool is full, the parent calls `waitpid(-1, ...)` before
starting the next child.  Each finished child is matched with its task so the
report names the right file or URL.

`make` starts one `gcc -c` child per source, waits for all of them, and only
links if every compilation succeeded.  Warnings are displayed but are not
errors, as required by the statement.

The `samples/success` directory compiles cleanly; `samples/failure` contains a
syntax error, a missing header and an undefined symbol for testing the
failure paths.
