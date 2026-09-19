# C++26 SFML Breakout (WIP / Post-Mortem)

C++26 と SFML 2.x を用いて Docker Container (WSL2/WSLg) 上で構築された 2D ブロック崩しゲームのプロトタイプです。
本プロジェクトは実験的開発として実施され、モダン C++26 とレガシーな C++ GUI ライブラリの組み合わせにおけるメモリ管理・ABI の課題を検証した段階で開発を休止（WIP）しています。

---

## プロジェクト概要

* **目的**: C++26 / SFML / DevContainer 環境でのグラフィックスアプリ開発およびゲームアーキテクチャの検証
* **開発環境**: 
  * Docker Dev Container (Ubuntu 24.04 / GCC 16 Toolchain)
  * WSL2 / WSLg (X11 Forwarding via Socket Mirroring)
  * C++26, CMake
  * SFML 2.6.x

---

## 実装済み機能

* **ゲーム物理 & 衝突検出**
  * パドルのマウス追従およびキーボード操作（Space と 左クリック で発射、Esc でリセット）
  * ボールと画面境界・パドル・ブロックの反射処理（入射角・交差差分ベース）
テム
* **UI & 状態管理**
  * スコア計算およびゲーム状態（Playing / Game Clear / Game Over）の遷移制御

---

## 技術的知見・課題（Post-Mortem）

本プロジェクトの開発過程において、最新の C++26 コンパイラ最適化・ABI と、SFML 内部のオブジェクト構造（`sf::RectangleShape`, `sf::Text`, `sf::Font` 等）の間で以下のような知見が得られました。

1. **データと描画用オブジェクトの完全分離 (Data / View Separation)**
   * `sf::RectangleShape` 等の SFML 描画クラスを `std::vector` 等の動的コンテナで直接保持・移動 (`std::move`) させると、内部の `sf::VertexArray` バッファアドレス不整合やコピー破壊が発生する。
   * データ（位置・状態フラグ）のみを保持し、描画時に単一の UI オブジェクトの位置を更新して描画（使い回し）することで非連続メモリ領域のバグを回避。
2. **`sf::Font` と `sf::Text` の参照整合性と生存期間 (Lifetime Management)**
   * `sf::Text` は `sf::Font` をポインタ参照で保持するため、`TextManager` 内での `sf::Font` の移動や再確保（`std::optional` や一時オブジェクトの代入 `sf::Text{}`）によって破棄済みアドレス参照（`invalid vptr`）が発生する。
   * `std::unique_ptr<sf::Font>` によるヒープ固定、および解体順序（デストラクタの逆順呼び出し）の厳密な設計が必要。
3. **C++26 / GCC 16 工具鎖と SFML (C++11/17 ABI) のミスマッチ**
   * 最新コンパイラでビルドされた自作コードと、既存の SFML 共有ライブラリ間で `sizeof(sf::Text)` 等のクラスサイズ定義の乖離が発生（432 bytes vs 368 bytes）。
   * これにより `AddressSanitizer` にて `new-delete-type-mismatch` が検出される。Pimpl パターン等による具象クラスの隠蔽が必要。
4. **その他、品質が低いことの問題**
   * 全体としてSFMLは例外を多用しており、noexceptなどの指定が十分であるとは言えない
   * Vector2など数理クラス/関数は設定値としても用いたいが、コンストラクタがconstexpr指定されていないためグローバルなコンパイル時定数として設定することが困難である。
   * [[nodiscard]]属性が付与されておらず、戻り値無視の危険がある。
   * explisit指定子が付与されておらず、暗黙型変換の危険がある。
   * デフォルトコンストラクタによる初期化は未代入のものが存在する場合があり、容易に未定義動作を引き起こす。

---

## 今後の展望

既存ライブラリの ABI 構造や内部隠蔽の限界を痛感したため、本プロジェクトの知見（型安全なリソース管理、モダン C++26 のメモリモデル）を活かし、**独自の C++26 向け 2D ゲームライブラリ / エンジンの開発** へと移行しようと思っています。

---

## ライセンス

MIT License