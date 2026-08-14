---
layout: home

hero:
  name: "OpenTTD-JRPM"
  text: "精品融合的 OpenTTD 增强版"
  tagline: 机车换挂 · 模块化机场 · 并行下载 · 整局感知 AI · 命令重放 · 蓝图 · 城镇分区
  image:
    src: /jrpm-hero.png
    alt: OpenTTD-JRPM
  actions:
    - theme: brand
      text: 🚀 快速开始
      link: /guide/build
    - theme: alt
      text: 📚 功能总览
      link: /features/overview
    - theme: alt
      text: 🔍 GitHub
      link: https://github.com/Maicarons/jrpm

features:
  - icon: 🚂
    title: 机车换挂（decouple）
    details: 列车解挂/挂载完整体系：解挂订单、路签转移、耦合长度与车速限制、双车头支持、NewGRF 耦合、解挂后两列车独立调度。
  - icon: 🛫
    title: 模块化机场（multitile-airport）
    details: 多格机场系统重构：air 类型体系、PBS 空中调度、YAPF 航空寻路、可改造机场布局（allow_modify_airports）。
  - icon: ⬇️
    title: 并行内容下载
    details: 多镜像源（逗号分隔可自定义）+ 多文件并行下载 + 失败自动切换镜像 + 并发数可配置。
  - icon: 🤖
    title: 整局感知 AI
    details: 保留 NoAI，新增 AIGlobal 全局 API（所有公司财务/车辆/评级/地图数据），带游戏设置接入控制与示例 AI。
  - icon: 💰
    title: 建造价格 Tooltip
    details: 建造铁轨/道路/地形时，鼠标上方实时显示估算建造成本，避免误点大工程。
  - icon: 📦
    title: 车辆自动分组
    details: 按共享订单/调度一键自动建组归组，组名自动取线路名；支持控制台命令与窗口按钮。
  - icon: 🧱
    title: 物体级建造高亮
    details: 车站、轨道、车库、机场、工业在放置/拖拽建造工具时显示实时物体预览（不再是简单的矩形）。
  - icon: 📐
    title: 蓝图系统
    details: 框选区域后 blueprint_copy，再到别处 blueprint_build 即可重建铁路、车库、隧道、桥梁、车站与信号（16 个内存槽位 + 旋转）。
  - icon: 🏘️
    title: 城镇分区 + growth_tiles 存档
    details: Tz0–Tz4 城镇分区与本月/上月房屋建造/拆除地块持久化到新的 GRWT 存档块（旧存档自动兼容）。
  - icon: ⏺️
    title: 命令记录与重放
    details: cmdrecord start/stop 记录所有已执行命令；cmdreplay 重放，使用 jrpm 自己的命令序列化，无需额外的命令对象层。
---

<div class="vp-doc cta-block">

### 💎 一份关于 jrpm 的简短介绍

OpenTTD-JRPM（jrpm）是基于 **JGR's Patchpack** 的精品 fork，融合了 **pulsexlb px-patch**（机车换挂、
多格模块化机场）、**OpenTTD-modded** 与 **cmclient** 的部分精选功能，并加入了 jrpm 自研的整局感知
AI、并行下载与车辆自动分组等能力。完整的版本对比与设计理念请参阅
[版本历史](/guide/version-history) 与 [项目介绍](/guide/intro)。

</div>