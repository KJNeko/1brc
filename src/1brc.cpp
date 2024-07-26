#include <cassert>
#include <ratio>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <chrono>
#include <filesystem>

using namespace std;

struct City {
    string name;
    double min = 99999;
    double max = -99999;
    double total = 0;
    int num = 0;

    City(string city_name) : name(city_name) {
    }

    void add_value(const double temp) {
        max = std::max(max, temp);
        min = std::min(min, temp);
        total += temp;
        num++;
    }

    // Not used yet but will be useful for dividing and conquering
    void merge(const City &C) {
        max = std::max(max, C.max);
        min = std::min(min, C.min);
        total += C.total;
        num += C.num;
    }

    double avg() { return total / num; }
};

struct Record {
    string name = "";
    double temp = 0.0;
};

struct Bucket {
    map<string, City> bucket{};

    void parse_record(const Record R) {
        auto X = bucket.find(R.name);

        if (X == bucket.end()) {
            City C = City(R.name);
            C.add_value(R.temp);
            bucket.insert({R.name, C});
        } else {
            X->second.add_value(R.temp);
        }
    }

    // Not used yet but will be useful for dividing and conquering
    void merge(const Bucket &B) {
        for (const auto &[name, C]: B.bucket) {
            auto X = bucket.find(name);

            if (X == bucket.end()) {
                bucket.insert({name, C});
            } else {
                X->second.merge(C);
            }
        }
    }

    // Gonna want to implement this if you want to actually check for correctness 
    // and compare with reference scripts
    void print() {
        return;
    }
};

Record split(const string &line) {
    Record R;

    int i = line.find_first_of(';');

    // This might be off by 1
    R.name = line.substr(0, i);
    R.temp = stod(line.substr(i + 1, line.length() - i + 1));

    return R;
};

int main() {
    auto start = chrono::steady_clock::now();

    // this could be an input parameter but I'm lazy
    // the reference scripts all by default hardcoded to read from "measurements.txt"
    string filename = "./1brc/measurements.txt";

    assert(std::filesystem::exists(filename));

    if (ifstream file(filename); file) {
        string line;
        int N = 0;

        Bucket B;

        while (getline(file, line)) {
            N++;

            Record R = split(line);
            B.parse_record(R);

            if (N % 100000 == 0) {
                cout << N << "\r" << flush;
            }
        }

        cout << "\n";

        auto end = chrono::steady_clock::now();
        chrono::duration<double, milli> dur = end - start;

        cout << dur.count() << "ms\n";
    } else
        throw std::runtime_error("Failed to open file");

    return 0;
};
