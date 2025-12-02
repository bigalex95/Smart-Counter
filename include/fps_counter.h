#pragma once
#include <vector>
#include <numeric>
#include <chrono>

class FPSCounter {
public:
    FPSCounter() : frame_count_(0) {}
    
    // Add a timing sample in milliseconds
    void addSample(float time_ms) {
        times_ms_.push_back(time_ms);
        frame_count_++;
    }
    
    // Get average FPS from all samples
    float getAverageFPS() const {
        if (times_ms_.empty()) return 0.0f;
        
        float avg_ms = std::accumulate(times_ms_.begin(), times_ms_.end(), 0.0f) / times_ms_.size();
        if (avg_ms == 0.0f) return 0.0f;
        
        return 1000.0f / avg_ms;
    }
    
    // Get instantaneous FPS from last sample
    float getInstantFPS() const {
        if (times_ms_.empty()) return 0.0f;
        
        float last_ms = times_ms_.back();
        if (last_ms == 0.0f) return 0.0f;
        
        return 1000.0f / last_ms;
    }
    
    // Get frame count
    int getFrameCount() const {
        return frame_count_;
    }
    
    // Clear all samples
    void reset() {
        times_ms_.clear();
        frame_count_ = 0;
    }

private:
    std::vector<float> times_ms_;
    int frame_count_;
};
