---
layout: home

hero:
  name: "OpenTTD-JRPM"
  text: "融合 jgrpp 与 pulsexlb 特性的 OpenTTD 增强版"
  tagline: 机车换挂 · 模块化机场 · 并行下载 · 整局感知 AI · 全新版本 jrpm-0.1.0
  image:
    src: /logo.svg
    alt: JRPM
  actions:
    - theme: brand
      text: 快速开始
      link: /guide/build
    - theme: alt
      text: 功能总览
      link: /features/overview
    - theme: alt
      text: GitHub
      link: https://github.com/Maicarons/OpenTTD-patches

features:
  - icon: 🚂
    title: 机车换挂（decouple）
    details: 列车解挂/挂载完整体系：解挂订单、路签转移、耦合长度与车速限制、双车头支持、NewGRF 耦合、解挂后两列车独立调度。
  - icon: 🛫
    title: 模块化机场（multitile-airport）
    details: 多格机场系统重构：air 类型体系、PBS 空中调度、YAPF 航空寻路，可改造机场布局（allow_modify_airports）。
  - icon: ⬇️
    title: 并行内容下载
    details: 多镜像源（逗号分隔可自定义）+ 多文件并行下载 + 失败自动切换镜像 + 并发数可配置。
  - icon: 🤖
    title: 整局感知 AI
    details: 保留 NoAI，新增 AIGlobal 全局 API（所有公司财务/车辆/评级/地图数据），带游戏设置接入控制。
  - icon: 💰
    title: 建造价格 Tooltip
    details: 建造铁轨/道路/地形时，鼠标上方实时显示估算建造成本。
  - icon: 📦
    title: 车辆自动分组
    details: 按共享订单/调度一键自动建组归组，组名自动取线路名；支持控制台命令与窗口按钮。
---
