import { defineConfig } from 'vitepress'

export default defineConfig({
  title: 'OpenTTD-JRPM',
  description: 'JRPM —— 融合 jgrpp 与 pulsexlb 特性的 OpenTTD 增强版：机车换挂、模块化机场、并行下载、整局感知 AI',
  lang: 'zh-CN',
  lastUpdated: true,
  cleanUrls: true,

  themeConfig: {
    logo: '/logo.svg',
    nav: [
      { text: '首页', link: '/' },
      { text: '指南', link: '/guide/intro' },
      { text: '功能', link: '/features/overview' },
      { text: '研究', link: '/research/project-structure' },
      { text: '合并', link: '/merge/jrpm-version-merge' },
      { text: '性能', link: '/performance/server-tuning' },
      { text: '开发', link: '/dev/getting-started' },
    ],

    sidebar: {
      '/guide/': [
        {
          text: '指南',
          items: [
            { text: '项目介绍', link: '/guide/intro' },
            { text: '构建与安装', link: '/guide/build' },
            { text: '联机指南', link: '/guide/multiplayer' },
            { text: '版本历史', link: '/guide/version-history' },
          ],
        },
      ],
      '/features/': [
        {
          text: '功能文档',
          items: [
            { text: '功能总览', link: '/features/overview' },
            { text: '1. 资源下载（多镜像 + 并行）', link: '/features/01-resource-download' },
            { text: '2. 服务器上限研究', link: '/features/02-server-caps' },
            { text: '3. 车辆自动分组', link: '/features/03-vehicle-autogroup' },
            { text: '4. 整局感知 AI 接口', link: '/features/05-global-ai' },
            { text: '行程历史（Trip History）', link: '/features/triphistory' },
            { text: '飞机滑行速度可调', link: '/features/plane-taxi-speed' },
            { text: '联机 UI 增强（位置/货运/观战）', link: '/features/ui-enhancements' },
            { text: '第三批规划：高亮 + 蓝图', link: '/features/highlight-blueprint-plan' },
            { text: '四/五批与 CM 命令难度调研', link: '/features/batch4-5-difficulty-research' },
            { text: '机车换挂（decouple）', link: '/features/decouple' },
            { text: '模块化机场（multitile-airport）', link: '/features/multitile-airport' },
          ],
        },
      ],
      '/research/': [
        {
          text: '研究',
          items: [
            { text: '项目结构与代码组织', link: '/research/project-structure' },
          ],
        },
      ],
      '/merge/': [
        {
          text: '版本合并',
          items: [
            { text: 'jrpm 版本合并总览', link: '/merge/jrpm-version-merge' },
          ],
        },
      ],
      '/performance/': [
        {
          text: '性能优化',
          items: [
            { text: '服务器性能调优', link: '/performance/server-tuning' },
          ],
        },
      ],
      '/dev/': [
        {
          text: '开发',
          items: [
            { text: '开发指南', link: '/dev/getting-started' },
            { text: '新增游戏命令', link: '/dev/add-command' },
            { text: '新增脚本 API', link: '/dev/add-script-api' },
            { text: '新增设置项', link: '/dev/add-setting' },
          ],
        },
      ],
    },

    outline: { level: [2, 3], label: '本页目录' },

    docFooter: { prev: '上一页', next: '下一页' },

    lastUpdated: {
      text: '最后更新于',
      formatOptions: { dateStyle: 'medium', timeStyle: 'short' },
    },

    search: {
      provider: 'local',
      options: {
        translations: {
          button: { buttonText: '搜索文档', buttonAriaLabel: '搜索文档' },
          modal: {
            noResultsText: '未找到相关结果',
            resetButtonTitle: '清除查询',
            footer: { selectText: '选择', navigateText: '切换', closeText: '关闭' },
          },
        },
      },
    },

    socialLinks: [
      { icon: 'github', link: 'https://github.com/Maicarons/OpenTTD-patches' },
    ],
  },
})
