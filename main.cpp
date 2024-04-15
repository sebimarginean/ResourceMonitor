#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <typeinfo>
 
class MemoryMonitor {
bool error;
public:
    MemoryMonitor(){
    error = false;
    }
    double getCurrentMemoryUsage() {
 
        if(!error){
        std::string line;
        std::ifstream memInfo("/proc/meminfo");
 
        long long totalMem = 0, freeMem = 0, buffers = 0, cached = 0;
 
        if (!memInfo.is_open()) {
            std::cerr << "Unable to open /proc/meminfo" << std::endl;
            error = 1;
            return -1;
        }
 
        while (std::getline(memInfo, line)) {
            std::istringstream ss(line);
            std::string memType;
            long long memValue;
 
            ss >> memType >> memValue;
 
            if (memType == "MemTotal:") {
                totalMem = memValue;
            } else if (memType == "MemFree:") {
                freeMem = memValue;
            } else if (memType == "Buffers:") {
                buffers = memValue;
            } else if (memType == "Cached:") {
                cached = memValue;
            }
        }
        memInfo.close();
 
        long long usedMem = totalMem - freeMem - buffers - cached;
        double memUsage = static_cast<double>(usedMem) / static_cast<double>(totalMem) * 100.0;
 
        return memUsage;
        }
 
        return -1;
    }
};
 
class CpuMonitor {
private:
    unsigned long long prevTotalUser = 0, prevTotalUserLow = 0, prevTotalSys = 0, prevTotalIdle = 0;
    bool error;
 
public:
    CpuMonitor(){
    error = false;
    }
    double getCurrentCpuUsage() {
 
    if(!error){
        std::string line;
        std::ifstream procStat("/proc/stat");
 
        unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;
 
        if (!procStat.is_open()) {
            std::cerr << "Unable to open /proc/stat" << std::endl;
            error = 1;
            return -1;
        }
 
        while (std::getline(procStat, line)) {
            if (line.substr(0, 3) == "cpu") {
                std::istringstream ss(line);
                ss >> line >> totalUser >> totalUserLow >> totalSys >> totalIdle;
                break;
            }
        }
        procStat.close();
 
        if (prevTotalUser != 0 || prevTotalUserLow != 0 || prevTotalSys != 0 || prevTotalIdle != 0) {
            unsigned long long totalDiff = (totalUser - prevTotalUser) + (totalUserLow - prevTotalUserLow) + (totalSys - prevTotalSys);
            unsigned long long idleDiff = totalIdle - prevTotalIdle;
            total = totalDiff + idleDiff;
 
            double cpuUsage = (totalDiff * 100.0 / total);
 
            prevTotalUser = totalUser;
            prevTotalUserLow = totalUserLow;
            prevTotalSys = totalSys;
            prevTotalIdle = totalIdle;
 
            return cpuUsage;
        }
 
 
 
        prevTotalUser = totalUser;
        prevTotalUserLow = totalUserLow;
        prevTotalSys = totalSys;
        prevTotalIdle = totalIdle;
 
        return 0;
        }
        return -1;
    }
};
 
 
class NetworkMonitor {
private:
    long long prevRxBytes = 0, prevTxBytes = 0;
    bool error;
public:
    NetworkMonitor(){
    error = false;
    }
    std::tuple<long long, long long> getCurrentNetworkUsage() {
    if(!error){
        std::string line;
        std::ifstream networkStatsFile("/proc/net/dev");
 
        long long rxBytes = 0, txBytes = 0;
 
        if (!networkStatsFile.is_open()) {
            std::cerr << "Unable to open /proc/net/dev" << std::endl;
            error = 1;
            return std::make_tuple(-1, -1);
        }
 
        while (std::getline(networkStatsFile, line)) {
            std::istringstream iss(line);
            std::string interface;
            long long rx, tx;
            int aux;
 
            if (iss >> interface >> rx >> aux >> aux >> aux >> aux >> aux >> aux >> aux >>tx) {
                if (interface == "enp0s3:") { 
                    rxBytes = rx;
                    txBytes = tx;
                    break;
                }
            }
        }
        networkStatsFile.close();
 	long long rxBytesDiff = rxBytes - prevRxBytes;
        long long txBytesDiff = txBytes - prevTxBytes;
 
        prevRxBytes = rxBytes;
        prevTxBytes = txBytes;
        return std::make_tuple(rxBytesDiff, txBytesDiff);
        }
        return std::make_tuple(-1, -1);
    }
};
 
class IOMonitor {
    bool error;
    long long prevReads;
    long long prevWrites;
 
public:
    IOMonitor() {
        error = false;
        prevReads = 0;
        prevWrites = 0;
    }
 
    std::tuple<long long, long long> getCurrentIOUsage() {
        if (!error) {
            std::string line;
            std::ifstream ioStats("/proc/diskstats");
 
            long long reads = 0, writes = 0;
 
            if (!ioStats.is_open()) {
                std::cerr << "Unable to open /proc/diskstats" << std::endl;
                error = true;
                return std::make_tuple(-1, -1);
            }
 
            while (std::getline(ioStats, line)) {
                std::istringstream iss(line);
                std::string device;
                int aux;
                long long readCompleted, writeCompleted;
 
                if (iss >> aux >> aux >> device >> readCompleted >> aux >> aux >> aux >> writeCompleted) {
                    if(device == "sda"){
 
                    reads = readCompleted - prevReads;
                    writes = writeCompleted - prevWrites;
                    prevReads = readCompleted;
                    prevWrites = writeCompleted;
 
                    return std::make_tuple(reads, writes);}
                }
            }
            ioStats.close();
 
        }
 
        return std::make_tuple(-1, -1);
    }
};
 
 
void plotMonitorResource(int duration) {
    if(duration == 60)
    	std::cout << "Wait for " << duration << " seconds (default time) to generate the graph\n";
    else std::cout <<"Wait for " << duration << " seconds to generate the graph\n";
    FILE* gnuplotPipeCpu = popen("gnuplot -persist", "w");
    FILE* gnuplotPipeMemory = popen("gnuplot -persist", "w");
    FILE* gnuplotPipeReceived = popen("gnuplot -persist", "w");
    FILE* gnuplotPipeTransmitted = popen("gnuplot -persist", "w");
    FILE* gnuplotPipeIOReads = popen("gnuplot -persist", "w");
    FILE* gnuplotPipeIOWrites = popen("gnuplot -persist", "w");
 
    if (gnuplotPipeCpu == nullptr || gnuplotPipeMemory == nullptr || gnuplotPipeReceived == nullptr || gnuplotPipeTransmitted == nullptr || gnuplotPipeIOReads == nullptr || gnuplotPipeIOWrites == nullptr) {
        std::cerr << "Unable to open gnuplot pipes\n";
        return;
    }
 
 
    fprintf(gnuplotPipeCpu, "set title 'CPU Usage'\n");
    fprintf(gnuplotPipeCpu, "set xlabel 'Time (s)'\n");
    fprintf(gnuplotPipeCpu, "set ylabel 'CPU Usage (%%)'\n");
    fprintf(gnuplotPipeCpu, "set key outside right\n");
    fprintf(gnuplotPipeCpu, "plot '-' title 'CPU Usage' with line linecolor rgb 'blue'\n");
 
 
    fprintf(gnuplotPipeMemory, "set title 'Memory Usage'\n");
    fprintf(gnuplotPipeMemory, "set xlabel 'Time (s)'\n");
    fprintf(gnuplotPipeMemory, "set ylabel 'Memory Usage (%%)'\n");
    fprintf(gnuplotPipeMemory, "set key outside right\n");
    fprintf(gnuplotPipeMemory, "plot '-' title 'Memory Usage' with line linecolor rgb 'blue'\n");
 
    fprintf(gnuplotPipeReceived, "set title 'Bandwidth Bytes Received'\n");
    fprintf(gnuplotPipeReceived, "set xlabel 'Time (s)'\n");
    fprintf(gnuplotPipeReceived, "set ylabel 'Bytes'\n");
    fprintf(gnuplotPipeReceived, "set key outside right\n");
    fprintf(gnuplotPipeReceived, "plot '-' title 'Bandwidth Bytes Received' with line linecolor rgb 'blue'\n");
 
    fprintf(gnuplotPipeTransmitted, "set title 'Bandwidth Bytes Transmitted'\n");
    fprintf(gnuplotPipeTransmitted, "set xlabel 'Time (s)'\n");
    fprintf(gnuplotPipeTransmitted, "set ylabel 'Bytes'\n");
    fprintf(gnuplotPipeTransmitted, "set key outside right\n");
    fprintf(gnuplotPipeTransmitted, "plot '-' title 'Bandwidth Bytes Transmitted' with line linecolor rgb 'blue'\n");
 
    fprintf(gnuplotPipeIOReads, "set title 'I/O Reads'\n");
    fprintf(gnuplotPipeIOReads, "set xlabel 'Time (s)'\n");
    fprintf(gnuplotPipeIOReads, "set ylabel 'Reads'\n");
    fprintf(gnuplotPipeIOReads, "set key outside right\n");
    fprintf(gnuplotPipeIOReads, "plot '-' title 'I/O Reads' with line linecolor rgb 'blue'\n");
 
    fprintf(gnuplotPipeIOWrites, "set title 'I/O Writes'\n");
    fprintf(gnuplotPipeIOWrites, "set xlabel 'Time (s)'\n");
    fprintf(gnuplotPipeIOWrites, "set ylabel 'Writes'\n");
    fprintf(gnuplotPipeIOWrites, "set key outside right\n");
    fprintf(gnuplotPipeIOWrites, "plot '-' title 'I/O Writes' with line linecolor rgb 'blue'\n");
 
    CpuMonitor cpu;
    MemoryMonitor memory;
    NetworkMonitor network;
    IOMonitor ioMonitor;
 
 
    double cpuUsage = cpu.getCurrentCpuUsage();
    double memoryUsage = memory.getCurrentMemoryUsage();
    auto [rxBytes, txBytes] = network.getCurrentNetworkUsage();
    auto [reads, writes] = ioMonitor.getCurrentIOUsage();
 
std::this_thread::sleep_for(std::chrono::milliseconds(500));
 
 
    for (int i = 0; i < duration; ++i) {
        int line = 0;
 
        if(cpuUsage != -1){
        line++;
        cpuUsage = cpu.getCurrentCpuUsage();
                std::cout << "CPU Usage at time " << i + 1 << "s: " << cpuUsage << "%  \n";
                        fprintf(gnuplotPipeCpu, "%d %lf\n", i + 1, cpuUsage);}
 
        if(memoryUsage != -1){
        line++;
        memoryUsage = memory.getCurrentMemoryUsage();
                std::cout << "Memory Usage at time " << i + 1 << "s: " << memoryUsage << "%  \n";
                        fprintf(gnuplotPipeMemory, "%d %lf\n", i + 1, memoryUsage);}
 
        if(rxBytes != -1){
        line += 2;
        auto [rxBytes, txBytes] = network.getCurrentNetworkUsage();
                std::cout << "Received Bytes at time " << i + 1 << "s: " << rxBytes << " bytes      \n";
                std::cout << "Transmitted Bytes at time " << i + 1 << "s: " << txBytes << " bytes      \n";
                fprintf(gnuplotPipeReceived, "%d %lld\n", i + 1, rxBytes);
                fprintf(gnuplotPipeTransmitted, "%d %lld\n", i + 1, txBytes);
        }
 
        if (reads != -1) {
            line += 2;
            auto [reads, writes] = ioMonitor.getCurrentIOUsage();
            std::cout << "I/O Reads at time " << i + 1 << "s: " << reads << "     \n";
            fprintf(gnuplotPipeIOReads, "%d %lld\n", i + 1, reads);
 
            std::cout << "I/O Writes at time " << i + 1 << "s: " << writes << "     \n";
            fprintf(gnuplotPipeIOWrites, "%d %lld\n", i + 1, writes);
        }
 
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "\e["+std::__cxx11::to_string(line)+"A\e[2K";
    }
 
    pclose(gnuplotPipeCpu);
    pclose(gnuplotPipeMemory);
    pclose(gnuplotPipeReceived);
    pclose(gnuplotPipeTransmitted);
    pclose(gnuplotPipeIOReads);
    pclose(gnuplotPipeIOWrites);
}
 
 
 
 
 
int main(int argc, char* argv[]) {
    int duration = 60;
 
    if (argc > 2) {
        std::cerr << "To many args. Use: ./proiect [duration]" << std::endl;
        return 1;
    } else if (argc == 2) {
    	if (std::isdigit(argv[1][0])) {
            try {
                duration = std::stoi(argv[1]);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid argument: " << argv[1] << std::endl;
                return 1;
            } catch (const std::out_of_range& e) {
                std::cerr << "Argument out of range: " << argv[1] << std::endl;
                return 1;
            }
        } else {
            std::cerr << "An integer value must be passed." << std::endl;
            return 1;
        }
    }
 
    plotMonitorResource(duration);
 
    return 0;
}