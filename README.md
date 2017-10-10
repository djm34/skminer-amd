SKMiner
=======
AMD GPU Miner for the SK1024 Algorithm used in the GPU Channel of Nexus. 


Created by Liam Russell A.K.A BitSlapper 

NXS: 2QscvkN2ddvKHhQZD9RjYS6nGXSLSSgQYJ6peCCXV8RNP8FZj2r


Optimized Kernel by djm34

NXS: 2S2pCpRXyb8Lpre52U3Xjq2MguSdaea5YGjVTsJqgZBfL2S24ag

Pooled by a.kubrakov
PoC purpose only! Should be used only as an example of miner-to-pool interaction

RUNNING
--------
setx GPU_FORCE_64BIT_PTR 1
SKMiner.exe

start SKMiner.exe without any arguments to use conf file
use resources/config/miner.conf file as an example
{
  "url" : "nexuspool.ru:8333",
  "user" : "WALLET_ADDRESS",
  "password" : "x"
}

BUILDING
---------

You will need these libraries to build from source:

boost 1.56.0, OpenSSL, json spirit, The C++ Rest SDK

You will also need the AMD ADL files. See the readme.txt in /source/ADL_SDK 

and of course the AMD APP SDK

Windows users can build from source right now, providing you have the libraries. I built using VS2017.

Linux users may have difficulty building from source currently.

Use https://github.com/HappinessInAutism/skminer-amd/releases if do not want to build it by yourself
