file(REMOVE_RECURSE
  "ArrayOpt/Output"
  "CXX/Output"
  "CXX/symex/Output"
  "CXX/symex/libc++/Output"
  "Concrete/Output"
  "Coverage/Output"
  "Dogfood/Output"
  "Expr/Output"
  "Expr/Lexer/Output"
  "Expr/Parser/Output"
  "Feature/Output"
  "Intrinsics/Output"
  "Merging/Output"
  "Programs/Output"
  "Replay/Output"
  "Replay/klee-replay/Output"
  "Replay/libkleeruntest/Output"
  "Runtime/Output"
  "Runtime/FreeStanding/Output"
  "Runtime/POSIX/Output"
  "Runtime/POSIX/SELinux/Output"
  "Runtime/Uclibc/Output"
  "Solver/Output"
  "VectorInstructions/Output"
  "regression/Output"
  "CMakeFiles/not.dir/not.cpp.o"
  "../bin/not.pdb"
  "../bin/not"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/not.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
