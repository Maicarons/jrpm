---
layout: home

hero:
  name: "OpenTTD-JRPM"
  text: "정선된 기능을 융합한 OpenTTD 강화판"
  tagline: 기관차 분리 · 모듈식 공항 · 병렬 다운로드 · 전판 인식 AI · 명령 재생 · 청사진 · 도시 구역
  image:
    src: /jrpm-hero.png
    alt: OpenTTD-JRPM
  actions:
    - theme: brand
      text: 🚀 빠른 시작
      link: /ko/guide/build
    - theme: alt
      text: 📚 기능 개요
      link: /ko/features/overview
    - theme: alt
      text: 🔍 GitHub
      link: https://github.com/Maicarons/jrpm

features:
  - icon: 🚂
    title: 기관차 분리 (decouple)
    details: " 열차 분리/결합 전체 체계: 분리 명령, 노선표 이전, 결합 길이와 속도 제한, 이중 기관차 지원, NewGRF 결합, 분리 후 두 열차 독립 운행."
  - icon: 🛫
    title: 모듈식 공항 (multitile-airport)
    details: " 다중 타일 공항 시스템 재구성: air 유형 체계, PBS 항공 관제, YAPF 항공 경로 탐색, 개조 가능한 공항 레이아웃 (allow_modify_airports)."
  - icon: ⬇️
    title: 병렬 콘텐츠 다운로드
    details: 다중 미러 (쉼표 구분, 사용자 정의 가능) + 다중 파일 병렬 다운로드 + 실패 시 자동 미러 전환 + 동시 실행 수 구성 가능.
  - icon: 🤖
    title: 전판 인식 AI
    details: NoAI 유지, 새로운 AIGlobal 전역 API (모든 회사 재무/차량/평가/지도 데이터), 게임 설정 접근 제어 및 예시 AI 포함.
  - icon: 💰
    title: 건설 비용 Tooltip
    details: 철로/도로/지형 건설 시 마우스 위에 실시간 예상 건설 비용 표시, 큰 공사 실수 방지.
  - icon: 📦
    title: 차량 자동 그룹화
    details: 공유 명령/일정에 따라 원클릭 자동 그룹 생성 및 배정, 그룹명은 자동으로 노선명 사용; 콘솔 명령과 창 버튼 지원.
  - icon: 🧱
    title: 객체 수준 건설 강조
    details: 역, 철로, 차고, 공항, 산업 시설 배치/드래그 건설 도구 사용 시 실시간 객체 미리보기 표시 (더 이상 단순한 사각형이 아님).
  - icon: 📐
    title: 청사진 시스템
    details: 영역 선택 후 blueprint_copy, 다른 곳에서 blueprint_build로 철로, 차고, 터널, 교량, 역 및 신호를 재건 (16개 메모리 슬롯 + 회전).
  - icon: 🏘️
    title: 도시 구역 + growth_tiles 저장
    details: Tz0–Tz4 도시 구역과 이번 달/지난 달 건물 건설/철거 타일이 새로운 GRWT 저장 청크에 영구 저장 (이전 저장과 자동 호환).
  - icon: ⏺️
    title: 명령 기록 및 재생
    details: cmdrecord start/stop으로 모든 실행 명령 기록; cmdreplay 재생, jrpm 자체 명령 직렬화 사용, 별도의 명령 객체 계층 불필요.
---

<div class="vp-doc cta-block">

### 💎 jrpm에 대한 간단한 소개

OpenTTD-JRPM (jrpm)은 **JGR's Patchpack**을 기반으로 한 정선된 fork로, **pulsexlb px-patch** (기관차 분리, 다중 타일 모듈식 공항), **OpenTTD-modded** 및 **cmclient**의 선별된 기능을 융합하고 jrpm 자체 개발의 전판 인식 AI, 병렬 다운로드, 차량 자동 그룹화 등의 기능을 추가했습니다. 전체 버전 비교와 디자인 철학은 [버전 역사](/ko/guide/version-history) 및 [프로젝트 소개](/ko/guide/intro)를 참조하세요.

</div>