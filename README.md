## CloudGame-RTSP-CGIServer-Yinli_Plan

云游戏将游戏资源、运行、内容的存储、计算和渲染都转移到云端，实时的游戏画面串流到终端进行显示，最终呈现到用户眼中。与本地游戏相比，云游戏增加了【抓屏、编码、网络传输、解码】等主要过程，即**流化过程**。

![云游戏基本原理示意图](photos\云游戏基本原理示意图.jpg)

**分类**

- x86 架构的云平台主要针对于 PC 端游和主机游戏的云化，ARM 架构的云平台主要针对手游的云化。
- 目前云平台的资源管理和调度有两种主流的方案——**【虚拟化方案】和【物理机方案】。**
  - **虚拟机流派**一般采用服务器和专业显卡的云端资源组合，并以虚拟化的方式分配资源，较为灵活。