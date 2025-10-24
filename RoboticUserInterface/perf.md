收集数据
> sudo perf record -g --call-graph=dwarf,8192 -F 99 ./build/Debug/robotic_user_interface

生成火焰图
> sudo perf script | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl > flamegraph.svg