// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"
#include <utility>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives


TEST(reg_10000_tools) {


    ToolRegistry r;
    Tool* first = nullptr;

    // adds 10000 tools with different names

    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        REQUIRE(r.add(std::make_unique<NamedTool>(name)));




        // remembers the first tool's address before any resizing

        if (i == 0) {
            first = r.find(name);
        }





        // checks the count and load factor after every add
        CHECK(r.size() == static_cast<std::size_t>(i + 1));
        CHECK(r.load_factor() <= 1.0);
    }




    // checks that every tool can still be found
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        CHECK(r.find(name) != nullptr);
    }





    // resizing should not change the first tool's address
    CHECK(first != nullptr);
    CHECK(r.find("tool_0") == first);





    // checks the longest chain against the spec's limit
    CHECK(r.max_probe_length() <= 64);




    // prints the measurements for our design log
    std::printf("buckets: %zu\n", r.bucket_count());
    std::printf("load factor: %.6f\n", r.load_factor());
    std::printf("longest chain: %zu\n", r.max_probe_length());

}



TEST(reg_alternative_hash) {
    ToolRegistry r;



    // builds the real table to get its final bucket count
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        REQUIRE(r.add(std::make_unique<NamedTool>(name)));
    }



    // counts how many names the weak hash puts in each bucket
    std::vector<int> counts(r.bucket_count(), 0);




    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);

        // only uses the first character and length of the name
        std::size_t weak_hash =
            static_cast<unsigned char>(name[0]) + name.size();

        std::size_t index = weak_hash % counts.size();
        ++counts[index];
    }



    // finds the longest chain produced by the weak hash

    int longest = 0;

    for (int count : counts) {
        if (count > longest) {
            longest = count;
        }
    }




    // compares both hashes using the same number of buckets
    std::printf("bucket count: %zu\n", r.bucket_count());
    std::printf("real hash longest chain: %zu\n", r.max_probe_length());
    std::printf("weak hash longest chain: %d\n", longest);

    CHECK(longest == 9000);
}



TEST(reg_null_rejected) {


    ToolRegistry r;




    // an empty registry should have no tools
    CHECK(r.size() == 0);
    CHECK(r.find("missing") == nullptr);




    // rejects an empty pointer without changing the count
    CHECK(!r.add(nullptr));
    CHECK(r.size() == 0);



    REQUIRE(r.add(std::make_unique<NamedTool>("alpha")));




    // rejecting another empty pointer should keep the existing tool
    CHECK(!r.add(nullptr));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") != nullptr);
}




TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;




    REQUIRE(r.add(std::make_unique<NamedTool>("alpha")));
    REQUIRE(r.add(std::make_unique<NamedTool>("beta")));
    REQUIRE(r.add(std::make_unique<NamedTool>("gamma")));




    int alpha = 0;
    int beta = 0;
    int gamma = 0;
    int total = 0;





    // counts each tool visited by for_each
    r.for_each([&](const Tool& tool) {
        ++total;

        if (tool.name() == "alpha") {
            ++alpha;
        } else if (tool.name() == "beta") {
            ++beta;
        } else if (tool.name() == "gamma") {
            ++gamma;
        }
    });




    // each registered tool should be visited exactly once
    CHECK(total == 3);
    CHECK(alpha == 1);
    CHECK(beta == 1);
    CHECK(gamma == 1);

}



TEST(ws_accepts_safe_paths) {


    TempDir d;
    Workspace ws(d.path());

    fs::path out;
    std::string reason;





    // each input is paired with its expected path inside the workspace
    std::vector<std::pair<std::string, std::string>> cases = {
        {"notes/todo.txt", "notes/todo.txt"},
        {"./notes//todo.txt", "notes/todo.txt"},
        {".", "."},
        {"a/../b", "b"},
        {"a/..", "."},
        {"new/dir/file.txt", "new/dir/file.txt"},
        {"my..notes.txt", "my..notes.txt"}
    };





    for (const auto& test : cases) {

        REQUIRE(ws.resolve(test.first, &out, &reason));




        // a dot should resolve to the root itself
        if (test.second == ".") {
            CHECK(out == ws.root());
        } else {
            CHECK(out == ws.root() / test.second);
        }

    }




    // resolving a path should not create anything
    CHECK(!fs::exists(d.path() / "new"));
}





TEST(ws_rejects_unsafe_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;



    std::vector<std::string> paths = {
        "",
        "/etc/passwd",
        "..",
        "../etc/passwd",
        "a/../../b",
        "notes/../../ws/notes/todo.txt"
    };





    for (const auto& path : paths) {

        // clears the reason so an old error cannot make this test pass
        reason.clear();

        CHECK(!ws.resolve(path, &out, &reason));
        CHECK(!reason.empty());
    }
}





TEST(ws_rejects_null_character) {


    TempDir d;
    Workspace ws(d.path());
    fs::path out;

    std::string reason;





    // builds the string explicitly so the null character is included

    std::string path = std::string("notes") + '\0' + "/x";

    CHECK(!ws.resolve(path, &out, &reason));
    CHECK(!reason.empty());

    
}



int main(int argc, char** argv) { return mt::run_all(argc, argv); }
