### Project 2 for CS475
To compile use
1. `clang -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp -lstdc++ main.cpp -o main`
2. `clang -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp -lstdc++ main.cpp -o main -lm`