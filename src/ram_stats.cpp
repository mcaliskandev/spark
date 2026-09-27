#include "ram_stats.hpp"

#include <fstream>
#include <iostream>
#include <string>

bool RamStats::ReadProcMeminfo(unsigned long long& mem_total_kb,
                               unsigned long long& mem_available_kb) const {
    std::ifstream file(proc_meminfo_path_);
    if (!file.is_open()) return false;

    bool mem_total_found = false;
    bool mem_available_found = false;

    std::string label;
    unsigned long long value;
    std::string unit;
    while (file >> label >> value >> unit) {
        if (label == mem_total_label_) {
            mem_total_kb = value;
            mem_total_found = true;
        } else if (label == mem_available_label_) {
            mem_available_kb = value;
            mem_available_found = true;
        }

        if (mem_total_found && mem_available_found) break;
    }

    return mem_total_found && mem_available_found;
}

void RamStats::Tick() {
    auto now = std::chrono::steady_clock::now();
    bool should_sample =
        !has_sampled_ || (now - last_sample_time_) >= sample_interval_;
    if (!should_sample) return;

    Update();
    last_sample_time_ = now;
    has_sampled_ = true;
}

bool RamStats::Update() {
    unsigned long long mem_total_kb = 0;
    unsigned long long mem_available_kb = 0;
    if (!ReadProcMeminfo(mem_total_kb, mem_available_kb)) {
        std::cerr << "Warning: failed to read " << proc_meminfo_path_ << "\n";
        return false;
    }

    total_kb_ = mem_total_kb;
    used_kb_ = mem_total_kb - mem_available_kb;

    usage_history_[history_offset_] = GetUsagePercent();
    history_offset_ = (history_offset_ + 1) % kHistorySize;

    return true;
}

float RamStats::GetUsagePercent() const {
    return total_kb_ == 0 ? 0.0f
                          : static_cast<float>(used_kb_) /
                                static_cast<float>(total_kb_) * 100.0f;
}

unsigned long long RamStats::GetTotalKB() const { return total_kb_; }

unsigned long long RamStats::GetUsedKB() const { return used_kb_; }

double RamStats::GetTotalGB() const {
    return static_cast<double>(total_kb_) / (1024.0 * 1024.0);
}

double RamStats::GetUsedGB() const {
    return static_cast<double>(used_kb_) / (1024.0 * 1024.0);
}

const float* RamStats::GetUsageHistory() const {
    return usage_history_.data();
}

int RamStats::GetHistorySize() const { return kHistorySize; }

int RamStats::GetHistoryOffset() const { return history_offset_; }
