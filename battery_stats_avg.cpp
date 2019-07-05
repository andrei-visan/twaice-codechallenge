#include <chrono>
#include <condition_variable>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>

#define AVG_INTERVAL 10
#define READER_SIM_WORK 20
#define WRITER_SIM_WORK 40

std::mutex mux;
std::condition_variable cond_var;

void read_csv(std::ifstream &csv_stream_in,
              std::queue<std::pair<float, float>> &q_readings,
              bool &work_done) {
    std::string csv_line;
    double voltage, temp;
    char separator;

    // skip csv header
    getline(csv_stream_in, csv_line);

    while (getline(csv_stream_in, csv_line)) {
        std::stringstream csv_line_ss(
            csv_line);  // make line string into stringstream

        csv_line_ss >> voltage >> separator >> temp;  // read values from line

        /* would use this for queue exclussive access
         *   in case of multiple producers
         */
        // std::unique_lock<std::mutex> lock(mux);

        q_readings.push(
            std::make_pair(voltage, temp));  // push values into queue

        std::cout << "Added data to queue\n";

        std::this_thread::sleep_for(
            std::chrono::milliseconds(READER_SIM_WORK));  // simulate extra work

        cond_var.notify_one();  // notify the writing thread
    }
    // shared variable marks reching the end of the data
    work_done = true;
}

void write_csv(std::ofstream &csv_stream_out,
               std::queue<std::pair<float, float>> &q_readings,
               bool &work_done) {
    // stores a (voltage, temperature) pair
    std::pair<float, float> value_pair;
    // stores at most the latest AVG_INTERVAL entries in the shared data queue
    std::queue<std::pair<float, float>> avg_buf;
    double sum_voltage = 0, sum_temp = 0;

    // while input data has not been fully consumed
    while (!work_done) {
        std::unique_lock<std::mutex> lock(mux);

        // while there is still data in the shared data queue
        if (q_readings.empty()) {
            cond_var.wait(lock);
        }
        while (!q_readings.empty()) {
            value_pair = q_readings.front();
            q_readings.pop();

            // add newest value to buffer
            avg_buf.push(value_pair);
            sum_voltage += value_pair.first;
            sum_temp += value_pair.second;

            // if buffer size has been exceeded, remove oldest value
            if (avg_buf.size() > AVG_INTERVAL) {
                sum_voltage -= avg_buf.front().first;
                sum_temp -= avg_buf.front().second;
                avg_buf.pop();
            }

            // outputing
            std::cout << sum_voltage / (double)avg_buf.size() << ','
                      << sum_temp / (double)avg_buf.size() << ','
                      << std::time(0) << '\n';

            csv_stream_out << sum_voltage / (double)avg_buf.size() << ','
                           << sum_temp / (double)avg_buf.size() << ','
                           << std::time(0) << '\n';

            std::this_thread::sleep_for(std::chrono::milliseconds(
                WRITER_SIM_WORK));  // simulate extra work
        }
    }
}

int main() {
    std::string data_input_file_path = "battery_data.csv";
    std::string data_output_file_path = "battery_average.csv";
    std::ifstream csv_stream_in(data_input_file_path);
    std::ofstream csv_stream_out(data_output_file_path);
    std::queue<std::pair<float, float>> q_readings;
    bool work_done = false;

    // check if input file was opened successfully
    if (csv_stream_in.fail()) {
        perror(data_input_file_path.c_str());
        return -1;
    }

    // check if output file was opened successfully
    if (csv_stream_out.fail()) {
        perror(data_output_file_path.c_str());
        return -1;
    }

    // start thread to read input and put it into shared data queue
    std::thread reader(read_csv, std::ref(csv_stream_in), std::ref(q_readings),
                       std::ref(work_done));
    // start thread to write the average of the read data from the shared queue
    std::thread writer(write_csv, std::ref(csv_stream_out),
                       std::ref(q_readings), std::ref(work_done));

    reader.join();
    writer.join();

    return 0;
}
