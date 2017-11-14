#include <thread>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <experimental/filesystem>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <signal.h>
#include <gtk/gtk.h>

#define PROC_PATH "/proc/"
#define WARN_THRESHOLD 1024000 //100MB

struct procinfo {
  int pid = 0;
  std::string name = "";
  long used_memory = 0; //expressed in Kilobytes
};

struct procinfo pcurrent;

bool is_digit(char c) {
  return ((c >= '0') && (c <= '9'));
}

struct procinfo get_proc_info(std::string path) {
  struct procinfo pinfo;
  std::ifstream input(path + "/status");
  for( std::string line; getline( input, line ); )
  {
    if (line.find("Name:") == 0) {
      int index = line.find_last_of('\t');
      pinfo.name = line.substr(index+1, line.length() - index);
    } else if (line.find("Pid:") == 0) {
      int index = line.find_last_of('\t');
      pinfo.pid = std::stoi(line.substr(index, line.length() - index));
    } else if (line.find("VmRSS:") == 0) {
      int index = line.find_last_of('\t');
      pinfo.used_memory = std::stol(line.substr(index, line.length() - index));
    }
  }
  if ((pinfo.pid == 0) || (pinfo.name == ""))
    throw std::runtime_error{"Error parsing process " + path + " " + pinfo.name + "\n"}; //pinfo.name could be not empty
  return pinfo;
}

void block_process(struct procinfo pinfo) {
  if (pinfo.pid > 1) { //avoid to crash the system
    if (kill(pinfo.pid, SIGKILL) != 0)
      throw std::runtime_error{"Signal SIGKILL failed"};
    //Show the notification
    std::system(std::string("notify-send 'Process killed' '" + pinfo.name + " was killed due to an high memory usage.'").c_str());
  }
}

void monitor(int seconds, int mem_threshold) {
  bool send_warn_message = true;
  while (true) {
    struct sysinfo sys_info;
    sysinfo(&sys_info);
    //int totalram = sys_info.totalram / 1024; //convertion from byte to Kilobyte
    int freeram = sys_info.freeram / 1024;
    for (const auto& p : std::experimental::filesystem::directory_iterator(PROC_PATH)) {
      if (is_digit(p.path().string().back())) { //Filter all the directories representing processes
        struct procinfo pinfo = get_proc_info(p.path().string());
        //Check for processes with a too high memory usage
        if (pinfo.used_memory > (mem_threshold * 1024)) {
          try {
            block_process(pinfo);
          } catch (const std::exception &e) {
            std::cerr << "Error in blocking process " << pinfo.pid << " (" << pinfo.name << ")\nDetails: " << e.what() << std::endl;
          }
        }
      }
    }
    if (freeram <= WARN_THRESHOLD) {
      if (send_warn_message == true) {
        std::system(std::string("notify-send 'WARNING' 'Memory almost saturated.\\nRunning processes: " + std::to_string(sys_info.procs) + "'").c_str());
        send_warn_message = false; //to avoid a flood of warnings
      }
    } else
      send_warn_message = true;
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
  }
}

int main(int argc, char const *argv[]) {
  if (argc < 3) {
    std::cout << "Usage:\n" << argv[0] << " <monitor_frequency_in_seconds> <used_memory_threshold>\n"
	    << "where all the processes that use more than <used_memory_threshold> MB of memory will be killed.\n\nExample:\n" << argv[0] << " 5 1024\n";
    return 0;
  }
  monitor(std::stoi(argv[1]), std::stoi(argv[2]));
  return 0;
}
