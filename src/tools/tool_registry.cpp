// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"
#include <cstdint>

ToolRegistry::ToolRegistry() : buckets_(8) {
}    // TODO: create your initial buckets
ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools





std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    // starts with the number given in the spec
    std::uint64_t h = 1469598103934665603ULL;

    // goes through every character in the tool name
    for (unsigned char c : key) {
        // mixes this character into the hash
        h ^= c;
        h *= 1099511628211ULL;
    }

    // returns the full hash, we choose the bucket later
    return static_cast<std::size_t>(h);
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {


    // rejects an empty pointer
    if (tool == nullptr) {
        return false;
    }



    // rejects a name that is already registered
    if (find(tool->name()) != nullptr) {
        return false;
    }



    // uses the name to choose a bucket

    std::size_t index = hash(tool->name()) % buckets_.size();



    // transfers ownership of the tool into this bucket
    buckets_[index].push_back(std::move(tool));



    // counts the new tool
    ++size_;
    return true;
}





Tool* ToolRegistry::find(std::string_view name) const {
    // uses the hash to choose a bucket

    std::size_t index = hash(name) % buckets_.size();

    // checks each tool in this bucket

    for (const auto& tool : buckets_[index]) {
        if (tool->name() == name) {
            // returns the pointer without giving up ownership
            return tool.get();
        }


    }

    // no tool in this bucket has that name
    return nullptr;
}




std::size_t ToolRegistry::size() const noexcept {
    // returns the number of registered tools
    return size_;
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {

    // goes through every bucket in the table

    for (const auto& bucket : buckets_) {

        // calls the given function on each tool in this bucket

        for (const auto& tool : bucket) {
            fn(*tool);
        }
    }

    
}


std::size_t ToolRegistry::bucket_count() const noexcept {
    // returns the number of buckets in the table
    return buckets_.size();
}


double ToolRegistry::load_factor() const noexcept {


    // divides the tool count by the bucket count
    // converts to double so we keep the decimal part

    return static_cast<double>(size_) / buckets_.size();
}

std::size_t ToolRegistry::max_probe_length() const {


    // starts at zero in case the table is empty

    std::size_t longest = 0;

    // checks how many tools are in each bucket


    for (const auto& bucket : buckets_) {
        if (bucket.size() > longest) {

            // remembers the longest chain found so far
            longest = bucket.size();
        }
    }

    return longest;
}
