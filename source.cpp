#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <omp.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sha1-1.h"

// using namespace std;

static std::string input_password_hash = "00c3cc7c9d684decd98721deaa0729d73faa9d9b";
static std::string salt = "parallel";
volatile bool found_password = false;

void check_hash(char* byte_array, size_t start, size_t end)
{
    size_t cursor = start;
    std::string line = "";
    while(cursor != end)
    {
        if(found_password != true)
        {
            if(byte_array[cursor] == '\n')
            {
                std::string file_password_hash = sha1(sha1(salt + line));
                if(input_password_hash.compare(file_password_hash) == 0)
                {
                    std::cout << "├─Found password: \"" << line << "\" -> " << file_password_hash << std::endl;
                    found_password = true;
                    return;
                }
                line = "";
            }
            else
            {
                line += byte_array[cursor];
            }
        }
        cursor++;
    }    
}

int main()
{
    auto start_time = std::chrono::high_resolution_clock::now();
    std::cout << std::endl << ". Sequential execution started!" << std::endl;
    
    std::ifstream ten_million("10-million-password-list-top-1000000.txt");

    if(ten_million.is_open())
    {
        std::string line;
        while(getline(ten_million, line))
        {
            std::string file_password_hash = sha1(sha1(salt + line));
            if(file_password_hash.compare(input_password_hash) == 0)
            {
                std::cout << "├─Found password: \"" << line << "\" -> " << file_password_hash << std::endl;
                break;
            }
        }
        ten_million.close();
    }

    auto finish_time = std::chrono::high_resolution_clock::now();
    std::cout << "└─Time taken: " << (std::chrono::duration_cast<std::chrono::microseconds>(finish_time - start_time)).count() << " milliseconds" << std::endl;



    // --------------------------------------------------



    start_time = std::chrono::high_resolution_clock::now();
    std::cout << std::endl << ". OpenMP parallel execution started!" << std::endl;

    int file_descriptor = open("10-million-password-list-top-1000000.txt", O_RDONLY);
    struct stat sb;
    fstat(file_descriptor, &sb);
    size_t file_size = sb.st_size;

    int num_threads = std::thread::hardware_concurrency();
    size_t chunk_size = file_size / num_threads;
    char *file_bytes = static_cast<char *>(mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0));

    #pragma omp parallel num_threads(num_threads)
    {
        int thread_id = omp_get_thread_num();
        size_t start = thread_id * chunk_size;
        size_t end = (thread_id == num_threads - 1) ? file_size : start + chunk_size;
        check_hash(file_bytes, start, end);
    }
    munmap(file_bytes, file_size);
    close(file_descriptor);

    finish_time = std::chrono::high_resolution_clock::now();
    std::cout << "└─Time taken: " << (std::chrono::duration_cast<std::chrono::microseconds>(finish_time - start_time)).count() << " milliseconds" << std::endl;

    return 0;
}
