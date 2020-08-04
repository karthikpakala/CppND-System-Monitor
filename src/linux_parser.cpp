#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/time.h>
#include <sstream>
#include <fstream>
#include "linux_parser.h"


using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  string version;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string line {""};
  string key {""};
  long memValue {0};
  float memTotalGB {0.0};
  float memFreeGB {0.0};
  float memPerc {0.0};
  string unit {""};
  string arr[4];
  arr[0] = "MemTotal:";
  arr[1] = "MemFree:";
  arr[2] = "MemAvailable:";
  arr[3] = "Buffers:";
  long outarr[4];
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);


  if(filestream.is_open()){
    int count {0};

    for (count = 0; count < 4; count ++){
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream>>key>>memValue>>unit;
      if(key == arr[count]){
        outarr[count] = memValue;
      }
    }
    memTotalGB = outarr[0]/(1024*1024);
    memFreeGB = outarr[1]/(1024*1024);
    memPerc = ((memTotalGB - memFreeGB)/memTotalGB);
  }

  return memPerc;
}


// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
  long uptime = 0;
  long idletime = 0;
  string line;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  std::getline(filestream, line);
  std::istringstream linestream(line);
  linestream>>uptime>>idletime;
  return uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::string line;
  std::string cpu{"cpu"};
  long user, nice, system, idle, iowait, irq, softirq, steal, guess, guessnice;
  long jiffies;
  if(filestream.is_open()){
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream>>cpu>>user>>nice>>system>>idle>>iowait>>irq>>softirq>>steal>>guess>>guessnice;
    long totalUserTime = user - guess;
    long totalNiceTime = nice - guessnice;
    long totalIdleTime = idle + iowait;
    long totalSystem = system + irq + softirq;
    long totalVirtualTime = guess + guessnice;
    jiffies =  totalUserTime + totalNiceTime + totalIdleTime + totalSystem + totalVirtualTime;
    return jiffies;
  }
  return jiffies; }
// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatFilename;
  std::ifstream filestream(filename.str());
  if (filestream.is_open()) {
      std::string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      std::string ignore;
      long utime;
      long stime;
      long cutime;
      long cstime;
      long starttime;
      for(int i = 0; i < 13; i++) linestream >> ignore;
      linestream >> utime >> stime >> cutime >> cstime ;
      for(int i = 0; i < 4; i++) linestream >> ignore;
      linestream >> starttime;
      return utime + stime + cutime + cstime +starttime;
  }
  return 0; 
  }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  long upTime = 0;
  long idleTime = 0;
  if(filestream.is_open()){
    std::string line;
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream>>upTime>>idleTime;
    return upTime;
  }
  return 0;
 }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  long idleTime = 0;
  long upTime = 0;
  if(filestream.is_open()){
    std::string line;
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream>>upTime>>idleTime;
    return idleTime;
  }
  return idleTime; 
}


// TODO: Read and return CPU utilization
float LinuxParser::CpuUtilization() { 
  std::ifstream filestream(kProcDirectory + kStatFilename);
  long prevuser{0}, prevnice{0}, prevsystem{0}, previdle{0}, previowait{0}, previrq{0}, prevsoftirq{0}, 
  prevsteal{0}, prevguest{0}, prevguestnice{0};
  std::string cpu {"cpu"};
  float cpuPerc{0.0};
  long Idle{0}, NonIdle{0}, Total{0}, System {0}, Virtual {0}, 
        PrevIdle{0}, PrevNonIdle{0}, PrevTotal{0}, PrevSystem {0}, PrevVirtual {0};
  if(filestream.is_open()){
    std::string line{""};
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream>>cpu>>prevuser>>prevnice>>prevsystem>>previdle>>previowait>>previrq>>prevsoftirq>>prevsteal>>prevguest>>prevguestnice;
    PrevIdle = previdle + previowait;
    PrevSystem = prevsystem + previrq + prevsoftirq;
    PrevVirtual = prevguest + prevguestnice;
    PrevNonIdle = prevuser + prevnice + PrevSystem + prevsteal;
    PrevTotal = PrevIdle + PrevNonIdle;
    filestream.close();
  }
    sleep(1);
  long user{0}, nice{0}, system{0}, idle{0}, iowait{0}, irq{0}, softirq{0}, steal{0}, guest{0}, guestnice{0};
  std::ifstream filestream1(kProcDirectory + kStatFilename);
  if(filestream1.is_open()){
    std::string line1 {""};
    std::getline(filestream1, line1);
    std::istringstream linestream1(line1);
    linestream1>>cpu>>user>>nice>>system>>idle>>iowait>>irq>>softirq>>steal>>guest>>guestnice;
    Idle = idle + iowait;
    System = system + irq + softirq;
    Virtual = guest + guestnice;
    NonIdle = user + nice + System  + steal;
    Total = Idle + NonIdle;
    filestream1.close();
  }
  float totalCpu = Total - PrevTotal;
  float totalCpuIdle = Idle - PrevIdle;
  cpuPerc = (totalCpu - totalCpuIdle)/totalCpu;
  return cpuPerc; 
  }

float LinuxParser::CpuUtilization(int pid){
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatFilename;
  std::ifstream filestream(filename.str());
  long cputime;
  if (filestream.is_open()) {
      std::string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      std::string ignore;
      long utime;
      long stime;
      long cutime;
      long cstime;
      long starttime;
      vector <long> values {0};
      /*
      for(int i = 0; i < 13; i++) linestream >> ignore;
      linestream >> utime >> stime >> cutime >> cstime ;
      for(int i = 0; i < 4; i++) linestream >> ignore;
      linestream >> starttime;
      cputime = utime + stime + cutime + cstime + starttime;
      return cputime;
  */
      for (int i = 0; i < 53; i++){
        int n = 0;
        long val;
        if(i > 13  || i < 18){
          linestream>>val;
          n++;
          values.at(n) = val;
        }
        
        else if(i = 22) {
          linestream >> val;
          starttime = val;
          break;
          
       }
      }
      utime = values.at(0);
      stime = values.at(1);
      cutime = values.at(2);
      cstime = values.at(3);
      long total_time = utime + stime + cutime + cstime;
      long uptime = LinuxParser::UpTime();
      //long Hertz = sysconf(_SC_CLK_TCK);
      long Hertz = 1;
      float seconds = uptime - (starttime / Hertz);

      cputime = 100 * ((total_time / Hertz) / seconds);

  }
  return cputime; 
  }



// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  std::ifstream filestream(kProcDirectory + kStatFilename);
  int totalProc {0};
  std::string key {""};
  //bool processFound = false;
  if(filestream.is_open()){
    std::string line;
    while(std::getline(filestream, line)){
    std::istringstream linestream(line);
    linestream>>key;
    if (key == "processes")
    {
      linestream >> totalProc;
     // return totalProc;
    }
    }
  }
  return totalProc;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  std::ifstream filestream(kProcDirectory + kStatFilename);
  int Proc {0};
  std::string key {""};
  //bool ProcBool = false;
  //bool processFound = false;
  if(filestream.is_open()){
    std::string line;
    //bool ProcBool = false;
    while(std::getline(filestream, line)){
    std::istringstream linestream(line);
    linestream>>key;
    if (key == "procs_running")
    {
      linestream >> Proc;
     // ProcBool = true;
     // return totalProc;
    }
    }
  }
  return Proc;
 }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function


string LinuxParser::Command(int pid) { 
 std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kCmdlineFilename;
  std::ifstream filestream(filename.str());
  std::string commandLine ;
  if (filestream.is_open()) {
      std::getline(filestream, commandLine);
  }
  return commandLine;
 }
// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { 
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatusFilename;
  std::ifstream filestream(filename.str());
  long memoryRam ;
  std::string unitRam;
  if (filestream.is_open()) {
      std::string line;
      bool foundRamMemory = false;
      while (!foundRamMemory && std::getline(filestream, line)) {
        std::istringstream linestream(line);
        std::string key;
        linestream >> key;
        if (key == "VmSize:") {
          linestream >> memoryRam >> unitRam;
          foundRamMemory = true;
        }

      }
  }
  std::ostringstream ostream;
  ostream << memoryRam/1024 ;
  return ostream.str(); 
  }
// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatusFilename;
  std::ifstream filestream(filename.str());
  string uid ;
  if (filestream.is_open()) {
      std::string line;
      bool foundUid = false;
      while (!foundUid && std::getline(filestream, line)) {
        std::istringstream linestream(line);
        std::string key;
        linestream >> key;
        if (key == "Uid:") {
          linestream >> uid;
          foundUid = true;
        }

      }
  }
  return uid; 
 }
// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) { 
  std::string uid = Uid(pid);
  std::string userName;
  std::ifstream filestream(kPasswordPath);
  // long runningProcesses = 0;
  if (filestream.is_open()) {
      std::string line;
      bool uidFound = false;
      
      while (std::getline(filestream, line) && !uidFound) {
        std::replace(line.begin(), line.end(), ' ', '_');
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);
        std::string pwd;
        std::string currentUid;
        linestream >> userName >> pwd >> currentUid;
        if (currentUid == uid)
        {          
            uidFound = true;
        }
      }
  }
  return userName; 
  }
// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatFilename;
  std::ifstream filestream(filename.str());
  long starttime = 0;
  if (filestream.is_open()) {
      std::string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      std::string ignore;
      for(int i = 0; i < 21; i++) linestream >> ignore;
      linestream >> starttime;
      struct timeval tv;
      gettimeofday(&tv, 0);
      //std::time_t now = std::time(0);
      std::time_t elapsedTime = LinuxParser::UpTime() - (starttime/sysconf(_SC_CLK_TCK));
      return elapsedTime;
  }
  return starttime; 
  }