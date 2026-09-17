#!/bin/bash
# Patch minicoro (vendored by BehaviorTree.CPP) for emscripten C++ builds:
# 1. Disable the em-fiber backend — em_asm.h is C++-only and breaks inside
#    minicoro's extern "C"; falling through selects the ASYNCIFY backend.
# 2. Rename the asyncify imports to emscripten's env-module builtins
#    (asyncify_start_unwind etc.), which the Asyncify runtime provides when
#    compiled with -sASYNCIFY=1.
f="$1"
sed -i.bak \
  -e 's/#elif defined(__EMSCRIPTEN__)/#elif 0/' \
  -e 's/import_module("asyncify"), import_name("start_unwind")/import_module("env"), import_name("asyncify_start_unwind")/' \
  -e 's/import_module("asyncify"), import_name("stop_unwind")/import_module("env"), import_name("asyncify_stop_unwind")/' \
  -e 's/import_module("asyncify"), import_name("start_rewind")/import_module("env"), import_name("asyncify_start_rewind")/' \
  -e 's/import_module("asyncify"), import_name("stop_rewind")/import_module("env"), import_name("asyncify_stop_rewind")/' \
  "$f"
