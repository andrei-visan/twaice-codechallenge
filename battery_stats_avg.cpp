#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <queue>

void read_csv(std::ifstream &csv_stream_in) {

    std::string csv_line;
    double voltage, temp;
    char separator;

    // skip csv header
    getline(csv_stream_in, csv_line);

    while (getline(csv_stream_in, csv_line)) {
        std::stringstream csv_line_ss(csv_line);
        csv_line_ss >> voltage >> separator >>  temp;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout<<voltage<<separator<<temp<<'\n';
    }
}


int main() {

    std::string data_file_path = "battery_data.csv";
    std::ifstream csv_stream_in(data_file_path);
    std::queue<std::pair<float, float>> q_values;

    // check if file was opened successfully
    if (csv_stream_in.fail()) {
        perror(data_file_path.c_str());
        return -1;
    }

    std::thread r(read_csv, std::ref(csv_stream_in));

    r.join();

    return 0;
}
