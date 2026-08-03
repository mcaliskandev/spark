#ifndef CPU_STATS_HPP
#define CPU_STATS_HPP

#include <chrono>
#include <vector>

class CpuStats {
   public:
    CpuStats();
    void Tick();
    float GetTotalUsage() const;
    const float GetCoreUsage(const int coreNumber) const;
    int GetCoreCount() const;

   private:
    struct CpuTimes {
        unsigned long long user = 0;
        unsigned long long nice = 0;
        unsigned long long system = 0;
        unsigned long long idle = 0;
        unsigned long long iowait = 0;
        unsigned long long irq = 0;
        unsigned long long softirq = 0;
        unsigned long long steal = 0;

        unsigned long long Idle() const { return idle + iowait; }
        unsigned long long Total() const {
            return user + nice + system + idle + iowait + irq + softirq + steal;
        }
    };

    bool Update();
    bool ReadProcStat(std::vector<CpuTimes>& out) const;
    static float ComputeUsage(const CpuTimes& prev, const CpuTimes& curr);

    float total_cpu_usage_ = 0.0f;
    std::vector<float> per_core_cpu_usage_;
    int total_core_number_{0};

    std::vector<CpuTimes> prev_times_;
    std::chrono::steady_clock::time_point last_sample_time_;
    bool has_sampled_ = false;
    static constexpr std::chrono::duration<double> sample_interval_{0.5};

    const char* proc_stat_path_ = "/proc/stat";
};

#endif  // CPU_STATS_HPP
