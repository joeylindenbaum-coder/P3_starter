// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.

#include "tools/workspace.h"

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {



    // rejects a path with no characters
    if (user_path.empty()) {
        *reason = "empty path";
        return false;
    }




    // rejects a null character anywhere in the path
    if (user_path.find('\0') != std::string_view::npos) {
        *reason = "path contains a null character";
        return false;
    }



    try {
        fs::path p{std::string(user_path)};




        // only allows paths relative to the workspace
        if (p.is_absolute() || p.has_root_path()) {
            *reason = "path must be relative";
            return false;
        }



        // removes extra slashes, dot components, and matching parent steps
        fs::path n = p.lexically_normal();




        // rejects a normalized path that goes above the workspace
        if (n.begin() != n.end() && *n.begin() == "..") {
            *reason = "path escapes the workspace";
            return false;
        }




        // a dot refers to the workspace root itself
        if (n == ".") {
            *out = root();
        } else {
            *out = root() / n;
        }




        return true;



        
    } catch (...) {
        // reports a path processing failure instead of throwing
        *reason = "could not resolve path";
        return false;
    }
}