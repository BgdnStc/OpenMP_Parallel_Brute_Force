C++ password brute-force in sequential approach and in parallel multi-threading execution using OpenMP.

Target SHA1 hash: 00c3cc7c9d684decd98721deaa0729d73faa9d9b
Prefix salt added: parallel

Compile: g++ -fopenmp source.cpp sha1-1.cpp -o ./bin/find_password
Run: ./bin/find_password