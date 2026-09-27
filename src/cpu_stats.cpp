#include "cpu_stats.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

CpuStats::CpuStats() {
    total_core_number_ = static_cast<int>(std::thread::hardware_concurrency());
    per_core_cpu_usage_.assign(total_core_number_, 0.0f);
}

bool CpuStats::ReadProcStat(std::vector<CpuTimes>& out) const {
    std::ifstream file(proc_stat_path_);
    if (!file.is_open()) return false;

    out.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("cpu", 0) != 0) break;

        std::istringstream iss(line);
        std::string label;
        iss >> label;

        CpuTimes t;
        iss >> t.user >> t.nice >> t.system >> t.idle >> t.iowait >> t.irq >>
            t.softirq >> t.steal;
        out.push_back(t);
    }

    return !out.empty();
}

float CpuStats::ComputeUsage(const CpuTimes& prev, const CpuTimes& curr) {
    unsigned long long prev_total = prev.Total();
    unsigned long long curr_total = curr.Total();
    unsigned long long total_delta = curr_total - prev_total;

    if (total_delta == 0) return 0.0f;

    unsigned long long idle_delta = curr.Idle() - prev.Idle();

    return static_cast<float>(total_delta - idle_delta) /
           static_cast<float>(total_delta) * 100.0f;
}

void CpuStats::Tick() {
    auto now = std::chrono::steady_clock::now();
    bool should_sample =
        !has_sampled_ || (now - last_sample_time_) >= sample_interval_;
    if (!should_sample) return;

    Update();
    last_sample_time_ = now;
    has_sampled_ = true;
}

bool CpuStats::Update() {
    std::vector<CpuTimes> curr_times;
    if (!ReadProcStat(curr_times)) {
        std::cerr << "Warning: failed to read " << proc_stat_path_ << "\n";
        return false;
    }

    if (prev_times_.size() == 0) {
        prev_times_ = curr_times;
        return true;
    }

    total_cpu_usage_ = ComputeUsage(prev_times_[0], curr_times[0]);
    for (size_t i = 0; i < total_core_number_; ++i) {
        per_core_cpu_usage_[i] = ComputeUsage(prev_times_[i], curr_times[i]);
    }

    usage_history_[history_offset_] = total_cpu_usage_;
    history_offset_ = (history_offset_ + 1) % kHistorySize;

    prev_times_ = curr_times;
    return true;
}

float CpuStats::GetTotalUsage() const { return total_cpu_usage_; }

const float CpuStats::GetCoreUsage(const int coreNumber) const {
    return per_core_cpu_usage_[coreNumber];
}

int CpuStats::GetCoreCount() const { return total_core_number_; }

const float* CpuStats::GetUsageHistory() const {
    return usage_history_.data();
}

int CpuStats::GetHistorySize() const { return kHistorySize; }

int CpuStats::GetHistoryOffset() const { return history_offset_; }
