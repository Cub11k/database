#include <random>
#include <chrono>
#include <sys/stat.h>

#include "test.h"

std::string random_string(bool non_zero = false)
{
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    std::random_device rd{};
    std::mt19937 generator(rd());

    std::string res{};
    if (non_zero) {
        res.push_back('a');
    }
    for (long unsigned i = 0; i < generator() % 100; ++i) {
        res.push_back(str[generator() % str.size()]);
    }
    return res;
}

long long random_group()
{
    std::random_device rd{};
    std::mt19937 generator(rd());

    return generator() % 1000;
}

double random_rating()
{
    std::random_device rd{};
    std::mt19937_64 generator(rd());
    std::uniform_real_distribution<double> unif(0, 5);
    return unif(generator);
}

void gen_test_file(const std::string &filename) {
    struct stat file_checker{};
    if (stat(filename.c_str(), &file_checker) == 0) {
        return;
    }
    std::ofstream fout(filename, std::ios::trunc | std::ios::out);
    if (!fout.is_open()) {
        std::cout << CANT_OPEN_FILE << std::endl;
        return;
    }
    fout << "{\n"
         << "\t\"students\": [\n";
    university::Student student{random_string(true), random_group(), random_rating(), random_string()};
    student.output(fout, "\t\t");
    for (int i = 0; i < 1000000; ++i) {
        student = {random_string(true), random_group(), random_rating(), random_string()};
        fout << ",\n";
        student.output(fout, "\t\t");
    }
    fout << "\n\t]\n}";
    std::cout << "File created" << std::endl;
    fout.close();
}

int test_load_from_file() {
    db::Database db{};
    gen_test_file("students.json");
    auto start = std::chrono::system_clock::now();
    // Some computation here;
    std::vector<std::string> result = db.process_query("LOAD file=load.json END", 0);
    for (auto &str: result) {
        std::cout << str;
        std::cout.flush();
    }
    std::cout << std::endl;
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed load time: " << elapsed_seconds.count() << "s" << std::endl;
    start = std::chrono::system_clock::now();
    result = db.process_query("DUMP file=dump.json END", 0);
    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;
    std::cout << "Elapsed dump time: " << elapsed_seconds.count() << "s" << std::endl;
    int ret = system("diff -s students.json dump.json");
    if (ret != 0) {
        return 1;
    }
    return 0;
}

int main()
{
    return test_load_from_file();
}
