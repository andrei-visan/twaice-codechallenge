## Task
Implement a program with two threads:

- One thread reads out the values from the battery_data.csv and sends them to the other thread.
- The other thread should get the average value of 10 data points and should store them with the UNIX timestamp in milliseconds in a second csv file

## Implementation

- The initial file and stream opening is managed by the main function, which starts the two threads: the "reader" and the "writer".
- The reader thread reads from the input file stream the csv values line by line and adds them to a queue shared with the writer thread. It signals when new data is available using a conditional_variable.
- The writer waits for data to be available in the shared queue. When work is available, it calculates a moving average over at most AVG_INTERVAL latest data points. If the queue has been emptied and the reader thread has not signaled via the "work_done" boolean flag that no new data will follow, it will wait to be notified via the conditional_variable.
