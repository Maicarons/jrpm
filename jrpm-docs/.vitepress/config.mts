import { defineConfig } from 'vitepress'

// ---------------------------------------------------------------------------
//  Helper: locale nav & sidebar definitions
// ---------------------------------------------------------------------------

interface NavItem {
  text: string
  link: string
}

interface SidebarGroup {
  text: string
  items: { text: string; link: string }[]
}

function makeLocaleConfig(
  lang: string,
  label: string,
  dir: 'ltr' | 'rtl',
  nav: NavItem[],
  sidebar: SidebarGroup[],
  outlineLabel: string,
  prev: string,
  next: string,
  lastUpdatedText: string,
  searchButton: string,
  searchNoResults: string,
  searchReset: string,
  searchSelect: string,
  searchNavigate: string,
  searchClose: string,
  tipLabel: string,
  warningLabel: string,
  dangerLabel: string,
  codeCopyTooltip: string,
  codeCopied: string,
) {
  // Build sidebar object (keyed by path prefix)
  const sidebarObj: Record<string, SidebarGroup[]> = {}
  for (const group of sidebar) {
    // Derive prefix from the first item's link
    const first = group.items[0]?.link ?? ''
    const prefix = '/' + first.split('/').filter(Boolean).slice(0, -1).join('/') + '/'
    if (!sidebarObj[prefix]) sidebarObj[prefix] = []
    sidebarObj[prefix].push(group)
  }

  return {
    label,
    lang,
    dir,
    link: lang === 'zh-CN' ? '/' : `/${lang === 'en' ? 'en' : lang}/`,
    themeConfig: {
      nav,
      sidebar: sidebarObj,
      outline: { level: [2, 3] as const, label: outlineLabel },
      docFooter: { prev, next },
      lastUpdated: { text: lastUpdatedText, formatOptions: { dateStyle: 'medium' as const, timeStyle: 'short' as const } },
      search: {
        provider: 'local' as const,
        options: {
          translations: {
            button: { buttonText: searchButton, buttonAriaLabel: searchButton },
            modal: {
              noResultsText: searchNoResults,
              resetButtonTitle: searchReset,
              footer: { selectText: searchSelect, navigateText: searchNavigate, closeText: searchClose },
            },
          },
        },
      },
    },
    markdown: {
      container: { tipLabel, warningLabel, dangerLabel },
      codeCopyButton: { tooltipText: codeCopyTooltip, copiedText: codeCopied },
    },
  }
}

// ---------------------------------------------------------------------------
//  Language data
// ---------------------------------------------------------------------------

const zhCN = makeLocaleConfig(
  'zh-CN', '简体中文', 'ltr',
  // nav
  [
    { text: '首页', link: '/' },
    { text: '指南', link: '/guide/intro' },
    { text: '功能', link: '/features/overview' },
    { text: '研究', link: '/research/project-structure' },
    { text: '合并', link: '/merge/jrpm-version-merge' },
    { text: '性能', link: '/performance/server-tuning' },
    { text: '开发', link: '/dev/getting-started' },
  ],
  // sidebar
  [
    {
      text: '指南',
      items: [
        { text: '项目介绍', link: '/guide/intro' },
        { text: '构建与安装', link: '/guide/build' },
        { text: '联机指南', link: '/guide/multiplayer' },
        { text: '版本历史', link: '/guide/version-history' },
      ],
    },
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
        { text: '高亮系统 + 蓝图系统', link: '/features/highlight-blueprint-plan' },
        { text: '城镇分区 + growth_tiles 存档', link: '/features/town-zoning' },
        { text: '命令记录与重放', link: '/features/command-replay' },
        { text: '功能难度调研（四/五批与 CM 命令）', link: '/features/batch4-5-difficulty-research' },
        { text: '机车换挂（decouple）', link: '/features/decouple' },
        { text: '模块化机场（multitile-airport）', link: '/features/multitile-airport' },
      ],
    },
    {
      text: '研究',
      items: [
        { text: '项目结构与代码组织', link: '/research/project-structure' },
      ],
    },
    {
      text: '版本合并',
      items: [
        { text: 'jrpm 版本合并总览', link: '/merge/jrpm-version-merge' },
      ],
    },
    {
      text: '性能优化',
      items: [
        { text: '服务器性能调优', link: '/performance/server-tuning' },
      ],
    },
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
  '本页目录', '上一页', '下一页', '最后更新于',
  '搜索文档', '未找到相关结果', '清除查询', '选择', '切换', '关闭',
  '提示', '警告', '危险', '复制代码', '已复制',
)

const en = makeLocaleConfig(
  'en', 'English', 'ltr',
  [
    { text: 'Home', link: '/en/' },
    { text: 'Guide', link: '/en/guide/intro' },
    { text: 'Features', link: '/en/features/overview' },
    { text: 'Research', link: '/en/research/project-structure' },
    { text: 'Merge', link: '/en/merge/jrpm-version-merge' },
    { text: 'Performance', link: '/en/performance/server-tuning' },
    { text: 'Development', link: '/en/dev/getting-started' },
  ],
  [
    {
      text: 'Guide',
      items: [
        { text: 'Introduction', link: '/en/guide/intro' },
        { text: 'Build & Install', link: '/en/guide/build' },
        { text: 'Multiplayer Guide', link: '/en/guide/multiplayer' },
        { text: 'Version History', link: '/en/guide/version-history' },
      ],
    },
    {
      text: 'Features',
      items: [
        { text: 'Feature Overview', link: '/en/features/overview' },
        { text: '1. Resource Download (Multi-Mirror + Parallel)', link: '/en/features/01-resource-download' },
        { text: '2. Server Capacity Research', link: '/en/features/02-server-caps' },
        { text: '3. Vehicle Auto-Grouping', link: '/en/features/03-vehicle-autogroup' },
        { text: '4. Global-Aware AI Interface', link: '/en/features/05-global-ai' },
        { text: 'Trip History', link: '/en/features/triphistory' },
        { text: 'Plane Taxi Speed', link: '/en/features/plane-taxi-speed' },
        { text: 'Multiplayer UI Enhancements', link: '/en/features/ui-enhancements' },
        { text: 'Highlight System + Blueprint System', link: '/en/features/highlight-blueprint-plan' },
        { text: 'Town Zoning + growth_tiles Save', link: '/en/features/town-zoning' },
        { text: 'Command Record & Replay', link: '/en/features/command-replay' },
        { text: 'Difficulty Research (Batch 4/5 & CM Cmds)', link: '/en/features/batch4-5-difficulty-research' },
        { text: 'Decouple (Locomotive Detachment)', link: '/en/features/decouple' },
        { text: 'Multitile Airport', link: '/en/features/multitile-airport' },
      ],
    },
    {
      text: 'Research',
      items: [
        { text: 'Project Structure & Code Organization', link: '/en/research/project-structure' },
      ],
    },
    {
      text: 'Version Merge',
      items: [
        { text: 'jrpm Version Merge Overview', link: '/en/merge/jrpm-version-merge' },
      ],
    },
    {
      text: 'Performance',
      items: [
        { text: 'Server Performance Tuning', link: '/en/performance/server-tuning' },
      ],
    },
    {
      text: 'Development',
      items: [
        { text: 'Getting Started', link: '/en/dev/getting-started' },
        { text: 'Adding Game Commands', link: '/en/dev/add-command' },
        { text: 'Adding Script APIs', link: '/en/dev/add-script-api' },
        { text: 'Adding Settings', link: '/en/dev/add-setting' },
      ],
    },
  ],
  'On this page', 'Previous', 'Next', 'Last updated',
  'Search docs', 'No results found', 'Clear query', 'Select', 'Navigate', 'Close',
  'Tip', 'Warning', 'Danger', 'Copy code', 'Copied',
)

const hi = makeLocaleConfig(
  'hi', 'हिन्दी', 'ltr',
  [
    { text: 'होम', link: '/hi/' },
    { text: 'गाइड', link: '/hi/guide/intro' },
    { text: 'विशेषताएँ', link: '/hi/features/overview' },
    { text: 'अनुसंधान', link: '/hi/research/project-structure' },
    { text: 'मर्ज', link: '/hi/merge/jrpm-version-merge' },
    { text: 'प्रदर्शन', link: '/hi/performance/server-tuning' },
    { text: 'डेवलपमेंट', link: '/hi/dev/getting-started' },
  ],
  [
    {
      text: 'गाइड',
      items: [
        { text: 'परिचय', link: '/hi/guide/intro' },
        { text: 'निर्माण और स्थापना', link: '/hi/guide/build' },
        { text: 'मल्टीप्लेयर गाइड', link: '/hi/guide/multiplayer' },
        { text: 'संस्करण इतिहास', link: '/hi/guide/version-history' },
      ],
    },
    {
      text: 'विशेषताएँ',
      items: [
        { text: 'विशेषता अवलोकन', link: '/hi/features/overview' },
        { text: '1. संसाधन डाउनलोड', link: '/hi/features/01-resource-download' },
        { text: '2. सर्वर क्षमता अनुसंधान', link: '/hi/features/02-server-caps' },
        { text: '3. वाहन ऑटो-ग्रुपिंग', link: '/hi/features/03-vehicle-autogroup' },
        { text: '4. वैश्विक AI इंटरफ़ेस', link: '/hi/features/05-global-ai' },
        { text: 'यात्रा इतिहास', link: '/hi/features/triphistory' },
        { text: 'विमान टैक्सी गति', link: '/hi/features/plane-taxi-speed' },
        { text: 'मल्टीप्लेयर UI संवर्द्धन', link: '/hi/features/ui-enhancements' },
        { text: 'हाइलाइट + ब्लूप्रिंट सिस्टम', link: '/hi/features/highlight-blueprint-plan' },
        { text: 'टाउन ज़ोनिंग', link: '/hi/features/town-zoning' },
        { text: 'कमांड रिकॉर्ड और रीप्ले', link: '/hi/features/command-replay' },
        { text: 'कठिनाई अनुसंधान', link: '/hi/features/batch4-5-difficulty-research' },
        { text: 'डिकपल (लोकोमोटिव डिटैचमेंट)', link: '/hi/features/decouple' },
        { text: 'मल्टीटाइल एयरपोर्ट', link: '/hi/features/multitile-airport' },
      ],
    },
    {
      text: 'अनुसंधान',
      items: [
        { text: 'प्रोजेक्ट संरचना', link: '/hi/research/project-structure' },
      ],
    },
    {
      text: 'संस्करण मर्ज',
      items: [
        { text: 'jrpm मर्ज अवलोकन', link: '/hi/merge/jrpm-version-merge' },
      ],
    },
    {
      text: 'प्रदर्शन',
      items: [
        { text: 'सर्वर प्रदर्शन ट्यूनिंग', link: '/hi/performance/server-tuning' },
      ],
    },
    {
      text: 'डेवलपमेंट',
      items: [
        { text: 'आरंभ करना', link: '/hi/dev/getting-started' },
        { text: 'गेम कमांड जोड़ना', link: '/hi/dev/add-command' },
        { text: 'स्क्रिप्ट API जोड़ना', link: '/hi/dev/add-script-api' },
        { text: 'सेटिंग जोड़ना', link: '/hi/dev/add-setting' },
      ],
    },
  ],
  'इस पृष्ठ पर', 'पिछला', 'अगला', 'अंतिम अपडेट',
  'दस्तावेज़ खोजें', 'कोई परिणाम नहीं', 'खोज साफ़ करें', 'चुनें', 'नेविगेट', 'बंद करें',
  'सुझाव', 'चेतावनी', 'खतरा', 'कोड कॉपी करें', 'कॉपी हुआ',
)

const es = makeLocaleConfig(
  'es', 'Español', 'ltr',
  [
    { text: 'Inicio', link: '/es/' },
    { text: 'Guía', link: '/es/guide/intro' },
    { text: 'Funciones', link: '/es/features/overview' },
    { text: 'Investigación', link: '/es/research/project-structure' },
    { text: 'Fusión', link: '/es/merge/jrpm-version-merge' },
    { text: 'Rendimiento', link: '/es/performance/server-tuning' },
    { text: 'Desarrollo', link: '/es/dev/getting-started' },
  ],
  [
    {
      text: 'Guía',
      items: [
        { text: 'Introducción', link: '/es/guide/intro' },
        { text: 'Compilación e Instalación', link: '/es/guide/build' },
        { text: 'Guía Multijugador', link: '/es/guide/multiplayer' },
        { text: 'Historial de Versiones', link: '/es/guide/version-history' },
      ],
    },
    {
      text: 'Funciones',
      items: [
        { text: 'Resumen de Funciones', link: '/es/features/overview' },
        { text: '1. Descarga de Recursos', link: '/es/features/01-resource-download' },
        { text: '2. Límites del Servidor', link: '/es/features/02-server-caps' },
        { text: '3. Agrupación Automática', link: '/es/features/03-vehicle-autogroup' },
        { text: '4. IA de Percepción Global', link: '/es/features/05-global-ai' },
        { text: 'Historial de Viajes', link: '/es/features/triphistory' },
        { text: 'Velocidad de Taxi de Aviones', link: '/es/features/plane-taxi-speed' },
        { text: 'Mejoras de UI Multijugador', link: '/es/features/ui-enhancements' },
        { text: 'Sistema de Resaltado + Planos', link: '/es/features/highlight-blueprint-plan' },
        { text: 'Zonificación Urbana', link: '/es/features/town-zoning' },
        { text: 'Grabación y Reproducción', link: '/es/features/command-replay' },
        { text: 'Investigación de Dificultad', link: '/es/features/batch4-5-difficulty-research' },
        { text: 'Desacople (Locomotoras)', link: '/es/features/decouple' },
        { text: 'Aeropuerto Multitile', link: '/es/features/multitile-airport' },
      ],
    },
    {
      text: 'Investigación',
      items: [
        { text: 'Estructura del Proyecto', link: '/es/research/project-structure' },
      ],
    },
    {
      text: 'Fusión de Versiones',
      items: [
        { text: 'Resumen de Fusión jrpm', link: '/es/merge/jrpm-version-merge' },
      ],
    },
    {
      text: 'Rendimiento',
      items: [
        { text: 'Optimización del Servidor', link: '/es/performance/server-tuning' },
      ],
    },
    {
      text: 'Desarrollo',
      items: [
        { text: 'Primeros Pasos', link: '/es/dev/getting-started' },
        { text: 'Agregar Comandos', link: '/es/dev/add-command' },
        { text: 'Agregar API de Script', link: '/es/dev/add-script-api' },
        { text: 'Agregar Configuraciones', link: '/es/dev/add-setting' },
      ],
    },
  ],
  'En esta página', 'Anterior', 'Siguiente', 'Última actualización',
  'Buscar documentos', 'Sin resultados', 'Limpiar búsqueda', 'Seleccionar', 'Navegar', 'Cerrar',
  'Consejo', 'Advertencia', 'Peligro', 'Copiar código', 'Copiado',
)

const fr = makeLocaleConfig(
  'fr', 'Français', 'ltr',
  [
    { text: 'Accueil', link: '/fr/' },
    { text: 'Guide', link: '/fr/guide/intro' },
    { text: 'Fonctionnalités', link: '/fr/features/overview' },
    { text: 'Recherche', link: '/fr/research/project-structure' },
    { text: 'Fusion', link: '/fr/merge/jrpm-version-merge' },
    { text: 'Performance', link: '/fr/performance/server-tuning' },
    { text: 'Développement', link: '/fr/dev/getting-started' },
  ],
  [
    {
      text: 'Guide',
      items: [
        { text: 'Introduction', link: '/fr/guide/intro' },
        { text: 'Compilation et Installation', link: '/fr/guide/build' },
        { text: 'Guide Multijoueur', link: '/fr/guide/multiplayer' },
        { text: 'Historique des Versions', link: '/fr/guide/version-history' },
      ],
    },
    {
      text: 'Fonctionnalités',
      items: [
        { text: 'Aperçu des Fonctionnalités', link: '/fr/features/overview' },
        { text: '1. Téléchargement de Ressources', link: '/fr/features/01-resource-download' },
        { text: '2. Limites du Serveur', link: '/fr/features/02-server-caps' },
        { text: '3. Regroupement Automatique', link: '/fr/features/03-vehicle-autogroup' },
        { text: '4. IA de Perception Globale', link: '/fr/features/05-global-ai' },
        { text: 'Historique des Trajets', link: '/fr/features/triphistory' },
        { text: 'Vitesse de Roulage des Avions', link: '/fr/features/plane-taxi-speed' },
        { text: 'Améliorations UI Multijoueur', link: '/fr/features/ui-enhancements' },
        { text: 'Système de Surbrillance + Plans', link: '/fr/features/highlight-blueprint-plan' },
        { text: 'Zonage Urbain', link: '/fr/features/town-zoning' },
        { text: 'Enregistrement et Rejeu', link: '/fr/features/command-replay' },
        { text: 'Recherche de Difficulté', link: '/fr/features/batch4-5-difficulty-research' },
        { text: 'Découplage (Locomotives)', link: '/fr/features/decouple' },
        { text: 'Aéroport Multitile', link: '/fr/features/multitile-airport' },
      ],
    },
    {
      text: 'Recherche',
      items: [
        { text: 'Structure du Projet', link: '/fr/research/project-structure' },
      ],
    },
    {
      text: 'Fusion de Versions',
      items: [
        { text: 'Aperçu Fusion jrpm', link: '/fr/merge/jrpm-version-merge' },
      ],
    },
    {
      text: 'Performance',
      items: [
        { text: 'Optimisation du Serveur', link: '/fr/performance/server-tuning' },
      ],
    },
    {
      text: 'Développement',
      items: [
        { text: 'Pour Commencer', link: '/fr/dev/getting-started' },
        { text: 'Ajouter des Commandes', link: '/fr/dev/add-command' },
        { text: 'Ajouter des API Script', link: '/fr/dev/add-script-api' },
        { text: 'Ajouter des Paramètres', link: '/fr/dev/add-setting' },
      ],
    },
  ],
  'Sur cette page', 'Précédent', 'Suivant', 'Dernière mise à jour',
  'Rechercher', 'Aucun résultat', 'Effacer la recherche', 'Sélectionner', 'Naviguer', 'Fermer',
  'Astuce', 'Avertissement', 'Danger', 'Copier le code', 'Copié',
)

const pt = makeLocaleConfig(
  'pt', 'Português', 'ltr',
  [
    { text: 'Início', link: '/pt/' },
    { text: 'Guia', link: '/pt/guide/intro' },
    { text: 'Funcionalidades', link: '/pt/features/overview' },
    { text: 'Pesquisa', link: '/pt/research/project-structure' },
    { text: 'Fusão', link: '/pt/merge/jrpm-version-merge' },
    { text: 'Desempenho', link: '/pt/performance/server-tuning' },
    { text: 'Desenvolvimento', link: '/pt/dev/getting-started' },
  ],
  [
    {
      text: 'Guia',
      items: [
        { text: 'Introdução', link: '/pt/guide/intro' },
        { text: 'Compilação e Instalação', link: '/pt/guide/build' },
        { text: 'Guia Multijogador', link: '/pt/guide/multiplayer' },
        { text: 'Histórico de Versões', link: '/pt/guide/version-history' },
      ],
    },
    {
      text: 'Funcionalidades',
      items: [
        { text: 'Visão Geral', link: '/pt/features/overview' },
        { text: '1. Download de Recursos', link: '/pt/features/01-resource-download' },
        { text: '2. Limites do Servidor', link: '/pt/features/02-server-caps' },
        { text: '3. Agrupamento Automático', link: '/pt/features/03-vehicle-autogroup' },
        { text: '4. IA de Percepção Global', link: '/pt/features/05-global-ai' },
        { text: 'Histórico de Viagens', link: '/pt/features/triphistory' },
        { text: 'Velocidade de Táxi de Aviação', link: '/pt/features/plane-taxi-speed' },
        { text: 'Melhorias de UI Multijogador', link: '/pt/features/ui-enhancements' },
        { text: 'Sistema de Destaque + Blueprint', link: '/pt/features/highlight-blueprint-plan' },
        { text: 'Zoneamento Urbano', link: '/pt/features/town-zoning' },
        { text: 'Gravação e Reprodução', link: '/pt/features/command-replay' },
        { text: 'Pesquisa de Dificuldade', link: '/pt/features/batch4-5-difficulty-research' },
        { text: 'Desacoplamento (Locomotivas)', link: '/pt/features/decouple' },
        { text: 'Aeroporto Multitile', link: '/pt/features/multitile-airport' },
      ],
    },
    {
      text: 'Pesquisa',
      items: [
        { text: 'Estrutura do Projeto', link: '/pt/research/project-structure' },
      ],
    },
    {
      text: 'Fusão de Versões',
      items: [
        { text: 'Visão Geral Fusão jrpm', link: '/pt/merge/jrpm-version-merge' },
      ],
    },
    {
      text: 'Desempenho',
      items: [
        { text: 'Otimização do Servidor', link: '/pt/performance/server-tuning' },
      ],
    },
    {
      text: 'Desenvolvimento',
      items: [
        { text: 'Primeiros Passos', link: '/pt/dev/getting-started' },
        { text: 'Adicionar Comandos', link: '/pt/dev/add-command' },
        { text: 'Adicionar API de Script', link: '/pt/dev/add-script-api' },
        { text: 'Adicionar Configurações', link: '/pt/dev/add-setting' },
      ],
    },
  ],
  'Nesta página', 'Anterior', 'Próximo', 'Última atualização',
  'Pesquisar documentos', 'Nenhum resultado', 'Limpar pesquisa', 'Selecionar', 'Navegar', 'Fechar',
  'Dica', 'Aviso', 'Perigo', 'Copiar código', 'Copiado',
)

const ar = makeLocaleConfig(
  'ar', 'العربية', 'rtl',
  [
    { text: 'الرئيسية', link: '/ar/' },
    { text: 'الدليل', link: '/ar/guide/intro' },
    { text: 'الميزات', link: '/ar/features/overview' },
    { text: 'البحث', link: '/ar/research/project-structure' },
    { text: 'الدمج', link: '/ar/merge/jrpm-version-merge' },
    { text: 'الأداء', link: '/ar/performance/server-tuning' },
    { text: 'التطوير', link: '/ar/dev/getting-started' },
  ],
  [
    {
      text: 'الدليل',
      items: [
        { text: 'مقدمة', link: '/ar/guide/intro' },
        { text: 'البناء والتثبيت', link: '/ar/guide/build' },
        { text: 'دليل متعدد اللاعبين', link: '/ar/guide/multiplayer' },
        { text: 'سجل الإصدارات', link: '/ar/guide/version-history' },
      ],
    },
    {
      text: 'الميزات',
      items: [
        { text: 'نظرة عامة على الميزات', link: '/ar/features/overview' },
        { text: '1. تنزيل الموارد', link: '/ar/features/01-resource-download' },
        { text: '2. حدود الخادم', link: '/ar/features/02-server-caps' },
        { text: '3. التجميع التلقائي للمركبات', link: '/ar/features/03-vehicle-autogroup' },
        { text: '4. واجهة AI العالمية', link: '/ar/features/05-global-ai' },
        { text: 'سجل الرحلات', link: '/ar/features/triphistory' },
        { text: 'سرعة سير الطائرات', link: '/ar/features/plane-taxi-speed' },
        { text: 'تحسينات واجهة متعدد اللاعبين', link: '/ar/features/ui-enhancements' },
        { text: 'نظام الإبراز + المخططات', link: '/ar/features/highlight-blueprint-plan' },
        { text: 'تقسيم المناطق', link: '/ar/features/town-zoning' },
        { text: 'تسجيل وإعادة الأوامر', link: '/ar/features/command-replay' },
        { text: 'بحث الصعوبة', link: '/ar/features/batch4-5-difficulty-research' },
        { text: 'فصل القاطرات', link: '/ar/features/decouple' },
        { text: 'مطار متعدد الخانات', link: '/ar/features/multitile-airport' },
      ],
    },
    {
      text: 'البحث',
      items: [
        { text: 'هيكل المشروع', link: '/ar/research/project-structure' },
      ],
    },
    {
      text: 'دمج الإصدارات',
      items: [
        { text: 'نظرة عامة على دمج jrpm', link: '/ar/merge/jrpm-version-merge' },
      ],
    },
    {
      text: 'الأداء',
      items: [
        { text: 'ضبط أداء الخادم', link: '/ar/performance/server-tuning' },
      ],
    },
    {
      text: 'التطوير',
      items: [
        { text: 'بدء التطوير', link: '/ar/dev/getting-started' },
        { text: 'إضافة أوامر اللعبة', link: '/ar/dev/add-command' },
        { text: 'إضافة واجهات Script API', link: '/ar/dev/add-script-api' },
        { text: 'إضافة إعدادات', link: '/ar/dev/add-setting' },
      ],
    },
  ],
  'في هذه الصفحة', 'السابق', 'التالي', 'آخر تحديث',
  'البحث في الوثائق', 'لا توجد نتائج', 'مسح البحث', 'اختيار', 'تنقل', 'إغلاق',
  'نصيحة', 'تحذير', 'خطر', 'نسخ الرمز', 'تم النسخ',
)

const bn = makeLocaleConfig(
  'bn', 'বাংলা', 'ltr',
  [
    { text: 'হোম', link: '/bn/' },
    { text: 'গাইড', link: '/bn/guide/intro' },
    { text: 'বৈশিষ্ট্য', link: '/bn/features/overview' },
    { text: 'গবেষণা', link: '/bn/research/project-structure' },
    { text: 'মার্জ', link: '/bn/merge/jrpm-version-merge' },
    { text: 'পারফরম্যান্স', link: '/bn/performance/server-tuning' },
    { text: 'ডেভেলপমেন্ট', link: '/bn/dev/getting-started' },
  ],
  [
    {
      text: 'গাইড',
      items: [
        { text: 'ভূমিকা', link: '/bn/guide/intro' },
        { text: 'বিল্ড ও ইনস্টলেশন', link: '/bn/guide/build' },
        { text: 'মাল্টিপ্লেয়ার গাইড', link: '/bn/guide/multiplayer' },
        { text: 'ভার্সন ইতিহাস', link: '/bn/guide/version-history' },
      ],
    },
    {
      text: 'বৈশিষ্ট্য',
      items: [
        { text: 'বৈশিষ্ট্য ওভারভিউ', link: '/bn/features/overview' },
        { text: '1. রিসোর্স ডাউনলোড', link: '/bn/features/01-resource-download' },
        { text: '2. সার্ভার সীমা গবেষণা', link: '/bn/features/02-server-caps' },
        { text: '3. যানবাহন অটো-গ্রুপিং', link: '/bn/features/03-vehicle-autogroup' },
        { text: '4. গ্লোবাল AI ইন্টারফেস', link: '/bn/features/05-global-ai' },
        { text: 'ভ্রমণ ইতিহাস', link: '/bn/features/triphistory' },
        { text: 'বিমান ট্যাক্সি গতি', link: '/bn/features/plane-taxi-speed' },
        { text: 'মাল্টিপ্লেয়ার UI উন্নতি', link: '/bn/features/ui-enhancements' },
        { text: 'হাইলাইট + ব্লুপ্রিন্ট সিস্টেম', link: '/bn/features/highlight-blueprint-plan' },
        { text: 'টাউন জোনিং', link: '/bn/features/town-zoning' },
        { text: 'কমান্ড রেকর্ড ও রিপ্লে', link: '/bn/features/command-replay' },
        { text: 'কঠিনতা গবেষণা', link: '/bn/features/batch4-5-difficulty-research' },
        { text: 'ডিকপল (লোকোমোটিভ বিচ্ছিন্নকরণ)', link: '/bn/features/decouple' },
        { text: 'মাল্টিটাইল এয়ারপোর্ট', link: '/bn/features/multitile-airport' },
      ],
    },
    {
      text: 'গবেষণা',
      items: [
        { text: 'প্রকল্প কাঠামো', link: '/bn/research/project-structure' },
      ],
    },
    {
      text: 'ভার্সন মার্জ',
      items: [
        { text: 'jrpm মার্জ ওভারভিউ', link: '/bn/merge/jrpm-version-merge' },
      ],
    },
    {
      text: 'পারফরম্যান্স',
      items: [
        { text: 'সার্ভার পারফরম্যান্স টিউনিং', link: '/bn/performance/server-tuning' },
      ],
    },
    {
      text: 'ডেভেলপমেন্ট',
      items: [
        { text: 'শুরু করুন', link: '/bn/dev/getting-started' },
        { text: 'গেম কমান্ড যোগ করুন', link: '/bn/dev/add-command' },
        { text: 'স্ক্রিপ্ট API যোগ করুন', link: '/bn/dev/add-script-api' },
        { text: 'সেটিং যোগ করুন', link: '/bn/dev/add-setting' },
      ],
    },
  ],
  'এই পৃষ্ঠায়', 'পূর্ববর্তী', 'পরবর্তী', 'সর্বশেষ আপডেট',
  'ডকুমেন্টস অনুসন্ধান', 'কোন ফলাফল নেই', 'অনুসন্ধান মুছুন', 'নির্বাচন', 'নেভিগেট', 'বন্ধ',
  'টিপ', 'সতর্কতা', 'বিপদ', 'কোড কপি করুন', 'কপি হয়েছে',
)

const ru = makeLocaleConfig(
  'ru', 'Русский', 'ltr',
  [
    { text: 'Главная', link: '/ru/' },
    { text: 'Руководство', link: '/ru/guide/intro' },
    { text: 'Возможности', link: '/ru/features/overview' },
    { text: 'Исследование', link: '/ru/research/project-structure' },
    { text: 'Слияние', link: '/ru/merge/jrpm-version-merge' },
    { text: 'Производительность', link: '/ru/performance/server-tuning' },
    { text: 'Разработка', link: '/ru/dev/getting-started' },
  ],
  [
    {
      text: 'Руководство',
      items: [
        { text: 'Введение', link: '/ru/guide/intro' },
        { text: 'Сборка и установка', link: '/ru/guide/build' },
        { text: 'Многопользовательский режим', link: '/ru/guide/multiplayer' },
        { text: 'История версий', link: '/ru/guide/version-history' },
      ],
    },
    {
      text: 'Возможности',
      items: [
        { text: 'Обзор возможностей', link: '/ru/features/overview' },
        { text: '1. Загрузка ресурсов', link: '/ru/features/01-resource-download' },
        { text: '2. Лимиты сервера', link: '/ru/features/02-server-caps' },
        { text: '3. Автогруппировка ТС', link: '/ru/features/03-vehicle-autogroup' },
        { text: '4. Глобальный AI интерфейс', link: '/ru/features/05-global-ai' },
        { text: 'История поездок', link: '/ru/features/triphistory' },
        { text: 'Скорость руления самолётов', link: '/ru/features/plane-taxi-speed' },
        { text: 'Улучшения UI мультиплеера', link: '/ru/features/ui-enhancements' },
        { text: 'Подсветка + Система чертежей', link: '/ru/features/highlight-blueprint-plan' },
        { text: 'Зонирование городов', link: '/ru/features/town-zoning' },
        { text: 'Запись и воспроизведение команд', link: '/ru/features/command-replay' },
        { text: 'Исследование сложности', link: '/ru/features/batch4-5-difficulty-research' },
        { text: 'Расцепка (локомотивы)', link: '/ru/features/decouple' },
        { text: 'Мультитайловый аэропорт', link: '/ru/features/multitile-airport' },
      ],
    },
    {
      text: 'Исследование',
      items: [
        { text: 'Структура проекта', link: '/ru/research/project-structure' },
      ],
    },
    {
      text: 'Слияние версий',
      items: [
        { text: 'Обзор слияния jrpm', link: '/ru/merge/jrpm-version-merge' },
      ],
    },
    {
      text: 'Производительность',
      items: [
        { text: 'Настройка сервера', link: '/ru/performance/server-tuning' },
      ],
    },
    {
      text: 'Разработка',
      items: [
        { text: 'Начало работы', link: '/ru/dev/getting-started' },
        { text: 'Добавление команд', link: '/ru/dev/add-command' },
        { text: 'Добавление Script API', link: '/ru/dev/add-script-api' },
        { text: 'Добавление настроек', link: '/ru/dev/add-setting' },
      ],
    },
  ],
  'На этой странице', 'Назад', 'Вперёд', 'Последнее обновление',
  'Поиск в документации', 'Ничего не найдено', 'Очистить поиск', 'Выбрать', 'Навигация', 'Закрыть',
  'Совет', 'Предупреждение', 'Опасно', 'Копировать код', 'Скопировано',
)

const id = makeLocaleConfig(
  'id', 'Bahasa Indonesia', 'ltr',
  [
    { text: 'Beranda', link: '/id/' },
    { text: 'Panduan', link: '/id/guide/intro' },
    { text: 'Fitur', link: '/id/features/overview' },
    { text: 'Riset', link: '/id/research/project-structure' },
    { text: 'Gabung', link: '/id/merge/jrpm-version-merge' },
    { text: 'Kinerja', link: '/id/performance/server-tuning' },
    { text: 'Pengembangan', link: '/id/dev/getting-started' },
  ],
  [
    {
      text: 'Panduan',
      items: [
        { text: 'Pengantar', link: '/id/guide/intro' },
        { text: 'Bangun & Instalasi', link: '/id/guide/build' },
        { text: 'Panduan Multipemain', link: '/id/guide/multiplayer' },
        { text: 'Riwayat Versi', link: '/id/guide/version-history' },
      ],
    },
    {
      text: 'Fitur',
      items: [
        { text: 'Ikhtisar Fitur', link: '/id/features/overview' },
        { text: '1. Unduh Sumber Daya', link: '/id/features/01-resource-download' },
        { text: '2. Batas Server', link: '/id/features/02-server-caps' },
        { text: '3. Pengelompokan Kendaraan', link: '/id/features/03-vehicle-autogroup' },
        { text: '4. Antarmuka AI Global', link: '/id/features/05-global-ai' },
        { text: 'Riwayat Perjalanan', link: '/id/features/triphistory' },
        { text: 'Kecepatan Taxi Pesawat', link: '/id/features/plane-taxi-speed' },
        { text: 'Peningkatan UI Multipemain', link: '/id/features/ui-enhancements' },
        { text: 'Sistem Sorotan + Cetak Biru', link: '/id/features/highlight-blueprint-plan' },
        { text: 'Zonasi Kota', link: '/id/features/town-zoning' },
        { text: 'Rekam & Putar Ulang Perintah', link: '/id/features/command-replay' },
        { text: 'Riset Kesulitan', link: '/id/features/batch4-5-difficulty-research' },
        { text: 'Decouple (Lepas Lokomotif)', link: '/id/features/decouple' },
        { text: 'Bandara Multitile', link: '/id/features/multitile-airport' },
      ],
    },
    {
      text: 'Riset',
      items: [
        { text: 'Struktur Proyek', link: '/id/research/project-structure' },
      ],
    },
    {
      text: 'Gabung Versi',
      items: [
        { text: 'Ikhtisar Gabung jrpm', link: '/id/merge/jrpm-version-merge' },
      ],
    },
    {
      text: 'Kinerja',
      items: [
        { text: 'Optimasi Server', link: '/id/performance/server-tuning' },
      ],
    },
    {
      text: 'Pengembangan',
      items: [
        { text: 'Memulai', link: '/id/dev/getting-started' },
        { text: 'Menambah Perintah', link: '/id/dev/add-command' },
        { text: 'Menambah API Script', link: '/id/dev/add-script-api' },
        { text: 'Menambah Pengaturan', link: '/id/dev/add-setting' },
      ],
    },
  ],
  'Di halaman ini', 'Sebelumnya', 'Selanjutnya', 'Terakhir diperbarui',
  'Cari dokumen', 'Tidak ada hasil', 'Hapus pencarian', 'Pilih', 'Navigasi', 'Tutup',
  'Tips', 'Peringatan', 'Bahaya', 'Salin kode', 'Disalin',
)

const ko = makeLocaleConfig(
  'ko', '한국어', 'ltr',
  [
    { text: '홈', link: '/ko/' },
    { text: '가이드', link: '/ko/guide/intro' },
    { text: '기능', link: '/ko/features/overview' },
    { text: '연구', link: '/ko/research/project-structure' },
    { text: '병합', link: '/ko/merge/jrpm-version-merge' },
    { text: '성능', link: '/ko/performance/server-tuning' },
    { text: '개발', link: '/ko/dev/getting-started' },
  ],
  [
    {
      text: '가이드',
      items: [
        { text: '소개', link: '/ko/guide/intro' },
        { text: '빌드 및 설치', link: '/ko/guide/build' },
        { text: '멀티플레이어 가이드', link: '/ko/guide/multiplayer' },
        { text: '버전 역사', link: '/ko/guide/version-history' },
      ],
    },
    {
      text: '기능',
      items: [
        { text: '기능 개요', link: '/ko/features/overview' },
        { text: '1. 리소스 다운로드', link: '/ko/features/01-resource-download' },
        { text: '2. 서버 용량 연구', link: '/ko/features/02-server-caps' },
        { text: '3. 차량 자동 그룹화', link: '/ko/features/03-vehicle-autogroup' },
        { text: '4. 전역 AI 인터페이스', link: '/ko/features/05-global-ai' },
        { text: '운행 기록', link: '/ko/features/triphistory' },
        { text: '비행기 지상 활주 속도', link: '/ko/features/plane-taxi-speed' },
        { text: '멀티플레이어 UI 개선', link: '/ko/features/ui-enhancements' },
        { text: '하이라이트 + 청사진 시스템', link: '/ko/features/highlight-blueprint-plan' },
        { text: '도시 구역 설정', link: '/ko/features/town-zoning' },
        { text: '명령 기록 및 재생', link: '/ko/features/command-replay' },
        { text: '난이도 연구', link: '/ko/features/batch4-5-difficulty-research' },
        { text: '디커플 (기관차 분리)', link: '/ko/features/decouple' },
        { text: '멀티타일 공항', link: '/ko/features/multitile-airport' },
      ],
    },
    {
      text: '연구',
      items: [
        { text: '프로젝트 구조', link: '/ko/research/project-structure' },
      ],
    },
    {
      text: '버전 병합',
      items: [
        { text: 'jrpm 병합 개요', link: '/ko/merge/jrpm-version-merge' },
      ],
    },
    {
      text: '성능',
      items: [
        { text: '서버 성능 튜닝', link: '/ko/performance/server-tuning' },
      ],
    },
    {
      text: '개발',
      items: [
        { text: '시작하기', link: '/ko/dev/getting-started' },
        { text: '게임 명령 추가', link: '/ko/dev/add-command' },
        { text: '스크립트 API 추가', link: '/ko/dev/add-script-api' },
        { text: '설정 추가', link: '/ko/dev/add-setting' },
      ],
    },
  ],
  '이 페이지에서', '이전', '다음', '마지막 업데이트',
  '문서 검색', '검색 결과 없음', '검색 지우기', '선택', '이동', '닫기',
  '팁', '경고', '위험', '코드 복사', '복사됨',
)

// ---------------------------------------------------------------------------
//  Main config
// ---------------------------------------------------------------------------

export default defineConfig({
  lang: 'zh-CN',
  base: '/jrpm/',
  lastUpdated: true,
  cleanUrls: true,

  head: [
    ['link', { rel: 'icon', type: 'image/svg+xml', href: '/logo.svg' }],
    ['meta', { name: 'theme-color', content: '#00b7ff' }],
    ['meta', { property: 'og:image', content: '/jrpm-hero.png' }],
    ['meta', { property: 'og:title', content: 'OpenTTD-JRPM · 文档站' }],
    ['meta', { property: 'og:description', content: 'JGR Patchpack 精品 fork —— 机车换挂 · 模块化机场 · 并行下载 · 整局感知 AI' }],
    ['meta', { name: 'twitter:card', content: 'summary_large_image' }],
  ],

  locales: {
    root: { ...zhCN, lang: 'zh-CN', link: '/' },
    en: { ...en, lang: 'en', link: '/en/' },
    hi: { ...hi, lang: 'hi', link: '/hi/' },
    es: { ...es, lang: 'es', link: '/es/' },
    ar: { ...ar, lang: 'ar', link: '/ar/' },
    fr: { ...fr, lang: 'fr', link: '/fr/' },
    bn: { ...bn, lang: 'bn', link: '/bn/' },
    pt: { ...pt, lang: 'pt', link: '/pt/' },
    ru: { ...ru, lang: 'ru', link: '/ru/' },
    id: { ...id, lang: 'id', link: '/id/' },
    ko: { ...ko, lang: 'ko', link: '/ko/' },
  },

  themeConfig: {
    logo: '/logo.svg',
    socialLinks: [
      { icon: 'github', link: 'https://github.com/Maicarons/jrpm' },
    ],
    outline: { level: [2, 3] },
  },
})