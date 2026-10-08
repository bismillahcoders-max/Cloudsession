# Lab 6 – FIRST & FOLLOW Sets

C program that computes and prints the FIRST and FOLLOW sets of every
non-terminal for one of the five grammars in the Lab 6 handout.

| File | What it is |
| --- | --- |
| `first_follow.c` | The program (C99, no dependencies) |
| `Makefile` | `make` builds it, `make run` runs it |
| `run_all.sh` | Builds and prints the results for all five grammars |
| `screenshots/` | One build screenshot and one run per grammar |

## Run on macOS

```bash
xcode-select --install        # once, if `cc --version` doesn't work
git clone -b claude/tender-newton-8t58ph https://github.com/bismillahcoders-max/Cloudsession.git
cd Cloudsession/lab6
make
./first_follow                # type 1-5 at the prompt
./run_all.sh                  # all five grammars at once
```

## Taking your own screenshots on a Mac

Run `./first_follow`, then press **Cmd + Shift + 4**, press **Space**, and
click the Terminal window. The PNG lands on your Desktop.
