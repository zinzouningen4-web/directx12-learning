# DirectX 12 学習記録

C++ と DirectX 12 を使い、描画の仕組みを一つずつ学ぶためのリポジトリです。
現在は、青い背景に頂点ごとの色を補間した三角形を描画するコードまで実装しています。

## 現在の進捗

- [x] Win32 ウィンドウの作成とメッセージループ
- [x] DirectX 12 Device とデバッグレイヤーの設定
- [x] Command Queue、Swap Chain、バックバッファ、RTV の作成
- [x] Command Allocator と Command List による描画命令の記録
- [x] Resource Barrier による描画・表示状態の切り替え
- [x] Fence による CPU と GPU の同期
- [x] HLSL の頂点シェーダーとピクセルシェーダーのコンパイル
- [x] Root Signature と Pipeline State Object の作成
- [x] `SV_VertexID` と `DrawInstanced` による三角形の描画

詳しい記録は [学習ログ](docs/PROGRESS.md) にまとめています。

## 開発環境

- Windows / x64
- Visual Studio の「C++ によるデスクトップ開発」ワークロード
- MSVC プラットフォームツールセット **v145**
- Windows SDK **10.0.26100.0**
- C++20
- DirectX 12 に対応した GPU とドライバー

プロジェクトの設定は `DirectX12Study.vcxproj` にあります。
Debug 実行時は DirectX 12 のデバッグレイヤーを使用するため、Windows のオプション機能「グラフィックス ツール」が必要です。

## ビルドと実行

1. このリポジトリをクローンします。
2. Visual Studio で `DirectX12Study.slnx` を開きます。対応していない環境では `DirectX12Study.vcxproj` を直接開いてください。
3. 構成を `Debug` または `Release`、プラットフォームを `x64` にしてビルドします。
4. Visual Studio から実行します。

Developer PowerShell では、次のコマンドでもビルドできます。

```powershell
msbuild .\DirectX12Study.vcxproj /m /p:Configuration=Debug /p:Platform=x64
```

生成される実行ファイルは `build/x64/Debug/DirectX12Study.exe` です。
Release の場合は `build/x64/Release/DirectX12Study.exe` です。

## ファイル構成

```text
src/Main.cpp              ウィンドウ、初期化、HLSL、描画ループ
DirectX12Study.vcxproj     Visual Studio プロジェクト
DirectX12Study.slnx        ソリューション
docs/PROGRESS.md           学習ログ
```

## 進捗の残し方

学習が一区切りついたら、コードと `docs/PROGRESS.md` を更新し、変更内容が分かるコミットを追加します。
学習ログには「実装したこと」「理解したこと」「次に試すこと」を記録します。

このリポジトリは学習用コードの公開スナップショットから開始しています。
ビルド生成物、Visual Studio の個人設定、個人用メモは含めていません。

## 現時点の制限

- 学習途中のため、描画処理は `src/Main.cpp` にまとめています。
- ウィンドウサイズ変更に伴うスワップチェーンの再作成は未実装です。
- 各フレームの最後に GPU の完了を待つ、理解しやすさを優先した同期方式です。
