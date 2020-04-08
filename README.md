# DLLinjector
A simple DLL injectior that injects a dll into another program's memory.

It firsts allocates the path of the DLL into the target process' memory, and uses CreateRemoteThread to start a thread of the function LoadLibraryA of kernell32.dll inside the target's process, which ultimately loads the dll.
