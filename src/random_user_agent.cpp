#include "random_user_agent.h"

#include <vector>
#include <string>
#include <random>
#include <numeric>
#include <utility>

RandomUserAgent::RandomUserAgent(int next_after)
    : rd()
    , engine(rd())
    , next_after_(next_after)
{
    available_indexes_.resize(user_agents_.size());
    std::iota(available_indexes_.begin(), available_indexes_.end(), 0);
    available_indexes_copy_for_quik_reset = available_indexes_;
    SetNextRandomIndex();
}

std::string RandomUserAgent::GetUserAgent() {
    if (uses_ >= next_after_) {
        SetNextRandomIndex();
    }
    ++uses_;
    return user_agents_[current_];
}

void RandomUserAgent::SetNextAfter(int next_after) {
    next_after_ = next_after;
}

void RandomUserAgent::SetNextRandomIndex() {
    if (available_indexes_.empty()) {
        available_indexes_ = available_indexes_copy_for_quik_reset;
    }

    std::uniform_int_distribution<size_t> dist(0, available_indexes_.size() - 1);
    size_t random_index = dist(engine);
    current_ = available_indexes_[random_index];
    std::swap(available_indexes_[random_index], available_indexes_.back());
    available_indexes_.pop_back();
}
