#ifndef RAM_STATS_HPP
#define RAM_STATS_HPP

#include <chrono>

class RamStats {
   public:
    void Tick();

    float GetUsagePercent() const;
    unsigned long long GetTotalKB() const;
    unsigned long long GetUsedKB() const;
    double GetTotalGB() const;
    double GetUsedGB() const;

   private:
    bool Update();
    bool ReadProcMeminfo(unsigned long long& mem_total_kb,
                         unsigned long long& mem_available_kb) const;

    unsigned long long total_kb_ = 0;
    unsigned long long used_kb_ = 0;

    std::chrono::steady_clock::time_point last_sample_time_;
    bool has_sampled_ = false;
    static constexpr std::chrono::duration<double> sample_interval_{0.5};

    const char* proc_meminfo_path_ = "/proc/meminfo";
    const char* mem_total_label_ = "MemTotal:";
    const char* mem_available_label_ = "MemAvailable:";
};

#endif  // RAM_STATS_HPP
