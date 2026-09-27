#ifndef DISK_STATS_HPP
#define DISK_STATS_HPP

#include <chrono>

class DiskStats {
   public:
    void Tick();

    float GetUsagePercent() const;
    unsigned long long GetTotalKB() const;
    unsigned long long GetUsedKB() const;
    double GetTotalGB() const;
    double GetUsedGB() const;

   private:
    bool Update();

    unsigned long long total_bytes_{0};
    unsigned long long used_bytes_{0};

    std::chrono::steady_clock::time_point last_sample_time_;
    bool has_sampled_ = false;
    static constexpr std::chrono::duration<double> sample_interval_{0.5};

    const char* disk_path_ = "/";
};

#endif  // DISK_STATS_HPP
