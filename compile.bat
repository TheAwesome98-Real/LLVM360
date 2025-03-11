C:/LLVM19/bin/llvm-as.exe output.ll -o input.bc
#C:/LLVM19/bin/opt.exe -passes="mem2reg,sroa,gvn" input.bc -o optimized.bc
#clang -O3 -target x86_64-pc-windows-msvc -fdeclspec -fms-extensions -fms-compatibility-version=19.42 -Wno-microsoft-cast -c optimized.bc -o input.o
C:/LLVM19/bin/opt.exe input.bc -o optimized.bc
clang -target x86_64-pc-windows-msvc -fdeclspec -fms-extensions -fms-compatibility-version=19.42 -Wno-microsoft-cast -c optimized.bc -o input.o

"C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.42.34433/bin/Hostx64/x64/link.exe" -out:output.exe -nologo input.o Runtime.lib "-libpath:C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.42.34433\\lib\\x64" "-libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.26100.0\\ucrt\\x64" "-libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.26100.0\\um\\x64" -defaultlib:libcmt -defaultlib:oldnames kernel32.lib user32.lib advapi32.lib ole32.lib shell32.lib uuid.lib ucrt.lib vcruntime.lib