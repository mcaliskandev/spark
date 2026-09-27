#include "disk_stats.hpp"

#include <sys/statvfs.h>

#include <iostream>

void DiskStats::Tick() {
    auto now = std::chrono::steady_clock::now();
    bool should_sample =
        !has_sampled_ || (now - last_sample_time_) >= sample_interval_;
    if (!should_sample) return;

    Update();
    last_sample_time_ = now;
    has_sampled_ = true;
}

bool DiskStats::Update() {
    struct statvfs stat;
    if (statvfs(disk_path_, &stat) != 0) {
        std::cerr << "Warning: failed to read disk stats for " << disk_path_
                  << "\n";
        return false;
    }

    unsigned long long block_size = stat.f_frsize;
    unsigned long long free_bytes = stat.f_bfree * block_size;

    total_bytes_ = stat.f_blocks * block_size;
    used_bytes_ = total_bytes_ - free_bytes;

    return true;
}

float DiskStats::GetUsagePercent() const {
    return total_bytes_ == 0 ? 0.0f
                             : static_cast<float>(used_bytes_) /
                                   static_cast<float>(total_bytes_) * 100.0f;
}

unsigned long long DiskStats::GetTotalKB() const { return total_bytes_ / 1000; }

unsigned long long DiskStats::GetUsedKB() const { return used_bytes_ / 1000; }

double DiskStats::GetTotalGB() const {
    return static_cast<double>(total_bytes_) / 1'000'000'000.0;
}

double DiskStats::GetUsedGB() const {
    return static_cast<double>(used_bytes_) / 1'000'000'000.0;
}
