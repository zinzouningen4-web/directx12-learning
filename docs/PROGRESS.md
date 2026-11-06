# 学習ログ

## 2026-11-06 — 三角形描画までのコードを公開

現在のソースコードを基に、DirectX 12 学習の進捗を記録する公開リポジトリを作成。
この日付は公開用の記録日であり、各機能を実装した日付を示すものではありません。

### 実装済み

- 1280 × 720 の描画領域を持つ Win32 ウィンドウとメッセージループ。
- Device、Command Queue、2 枚のバッファを持つ Swap Chain、RTV の初期化。
- Command List に描画命令を記録し、GPU に送信する流れ。
- バックバッファの Resource Barrier と `Present`。
- Fence と Windows イベントによる GPU 完了待ち。
- Debug 構成でのデバッグレイヤーと Info Queue のエラー検出設定。
- 埋め込み HLSL を `D3DCompile` でコンパイルする処理。
- Root Signature、PSO、Viewport、Scissor Rect の設定。
- `SV_VertexID` から 3 頂点の位置・色を生成し、`DrawInstanced(3, 1, 0, 0)` で描画する処理。

### コードから確認できる学習ポイント

- CPU が記録したコマンドを、Command Queue 経由で GPU に実行させる。
- 描画前後にバックバッファのリソース状態を切り替える。
- Fence で完了を確認してから、Command Allocator を再利用する。
- 頂点シェーダーの出力をピクセルシェーダーへ渡し、色を補間する。

### 確認結果

- MSVC v145 / Windows SDK 10.0.26100.0 で x64 の Debug・Release 両構成のビルド成功を確認。
- 今回の公開作業では、実行画面の目視確認は行っていません。

### 次の学習候補

- 頂点バッファと Input Layout を使う描画。
- 定数バッファによる座標変換。
- シェーダーの外部ファイル化。
- ウィンドウサイズ変更への対応。

今後の記録は、このファイルの上部へ日付ごとに追記します。
