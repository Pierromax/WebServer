# WebServer
Our first HTTP Server

## Configuration File Parsing

Our configuration parser faithfully reproduces nginx's behavior with identical error messages and directive handling. We used [test configuration files](https://github.com/42mates/webserv/tree/main/tools/config/tests) created by [Mbecker (marinsucks)](https://github.com/marinsucks) and developed a [**test script**](/test.sh) that automatically validates our parser against these files.

```bash
./test.sh
```
_To run the tests_
