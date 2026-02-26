# FbxAxisUnitConverter

FBX ファイルの **軸系**（Up/Forward ベクトル）と**単位**（cm, m など）を変換する C++ CLI ツールです。
変換は座標値を直接書き換える（Swizzle）方式で行うため、追加ノードや変換ノードを挿入せず、DCC ツール上の見た目と Transform 値の整合性を保ちます。

## 機能

- **軸変換** — 任意の Up/Forward ベクトルの組み合わせに変換（例: Y-up/-Z-forward → Z-up/Y-forward）
- **単位変換** — FBX SDK 定義の単位（mm / cm / dm / m / km / inch / foot / yard / mile）へのスケール変換
- **ジオメトリ焼き込み** — 頂点、法線、Tangent、Binormal の座標変換
- **ノード Transform 変換** — LclTranslation / LclRotation / LclScaling の書き換え
- **共有メッシュ対応** — インスタンス化されたメッシュの重複変換を防止
- **入力軸/単位の自動検出** — FBX の GlobalSettings から読み取り（引数で上書き可能）
- **Pre-normalization** — Blender 等の DCC ツールがルートオブジェクトに焼き込んだ補正変換を除去してから通常変換を行う

## ビルド要件

| 項目 | バージョン |
|------|-----------|
| Autodesk FBX SDK | 2020.x 以降（スタティックリンク） |
| Visual Studio | 2022 (Build Tools v143) |
| C++ 規格 | C++17 |
| プラットフォーム | x64 |

FBX SDK は `C:\Program Files\Autodesk\FBX\FBX SDK\2020.x.x\` にインストールされている必要があります。
パスは `FbxAxisUnitConverter.vcxproj` 内の `<FbxSdkDir>` プロパティで変更できます。

## ビルド

```powershell
& 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe' `
    FbxAxisUnitConverter.sln /p:Configuration=Release /p:Platform=x64 /v:minimal
```

出力: `x64\Release\FbxAxisUnitConverter.exe`

FBX SDK はスタティックリンクのため、実行時に追加の DLL は不要です。

## 使い方

```
FbxAxisUnitConverter -i <input.fbx> -o <output.fbx> [options]

Options:
  -i, --input   <path>   入力 FBX ファイルパス（必須）
  -o, --output  <path>   出力 FBX ファイルパス（必須）
  --up          <axis>   出力の Up ベクトル         例: Y, Z, -Z
  --forward     <axis>   出力の Forward ベクトル    例: -Z, Y
  --unit        <unit>   出力の単位  mm|cm|dm|m|km|inch|foot|yard|mile
  --src-up      <axis>   入力の Up ベクトルを上書き
  --src-forward <axis>   入力の Forward ベクトルを上書き
  --src-unit    <unit>   入力の単位を上書き

Pre-normalization（--pre-norm-* と --src-* は排他）:
  --pre-norm-up      <axis>   補正前の実効 Up 軸    例: Z
  --pre-norm-forward <axis>   補正前の実効 Forward 軸  例: Y
  --pre-norm-unit    <unit>   補正前の実効単位       例: m
```

`--up` と `--forward` は **両方同時に指定するか、両方省略**してください。
Up ベクトルと Forward ベクトルは**直交**していなければなりません。
`--pre-norm-up` と `--pre-norm-forward` も同様のルールが適用されます。

### 使用例

```bash
# Y-up / -Z-forward（OpenGL/Unity）へ変換、単位を m に
FbxAxisUnitConverter -i model.fbx -o out.fbx --up Y --forward -Z --unit m

# Z-up / Y-forward（3ds Max / Maya Z-up 系）へ変換
FbxAxisUnitConverter -i model.fbx -o out.fbx --up Z --forward Y

# 単位だけ cm → m に変換（軸変換なし）
FbxAxisUnitConverter -i model.fbx -o out.fbx --unit m

# 入力 FBX の GlobalSettings が壊れている場合に軸を強制指定
FbxAxisUnitConverter -i model.fbx -o out.fbx \
    --src-up Y --src-forward -Z \
    --up Z --forward Y

# Blender デフォルト出力（メタデータ: Y-up/cm、実態: Z-up/m）を Y-up/cm に変換
FbxAxisUnitConverter -i blender_export.fbx -o out.fbx \
    --pre-norm-up Z --pre-norm-forward Y --pre-norm-unit m \
    --up Y --forward -Z --unit cm
```

### Forward ベクトルの定義

`--forward` は「キャラクターやオブジェクトが向く方向（画面奥方向）」を指定します。

| エンジン/ツール | --up | --forward |
|----------------|------|-----------|
| Unity / OpenGL | Y    | -Z        |
| Blender        | Z    | Y         |
| 3ds Max        | Z    | Y         |
| Maya (Y-up)    | Y    | -Z        |
| Maya (Z-up)    | Z    | -Y        |

### Pre-normalization について

Blender などの DCC ツールは、内部座標系（例: Z-up, m）と異なる設定で FBX をエクスポートする場合、FBX メタデータの軸/単位を書き換えたうえで、シーン内の各ルートオブジェクトに補正変換（LclRotation, LclScaling など）を焼き込みます。

`--pre-norm-*` オプションを使うと、変換パイプラインの先頭でその補正変換を自動的に除去し、その後の通常変換が正しい実効軸/単位を起点として動作するようになります。

**Blender の FBX エクスポート（デフォルト設定）の場合:**
- メタデータ上の軸系: Y-up / cm
- 実際のモデルは Z-up / m で作られており、各ルートオブジェクトに `LclRotation=(-90, 0, 0)`, `LclScaling=(100, 100, 100)` が焼き込まれている

```bash
FbxAxisUnitConverter -i blender_export.fbx -o out.fbx \
    --pre-norm-up Z --pre-norm-forward Y --pre-norm-unit m \
    --up Y --forward -Z --unit cm
```

## 制限事項

- **左手座標系**（DirectX 等）への変換は非対応（右手座標系のみ）
- **アニメーションカーブ**の変換は未実装（TODO）
- **スキンメッシュ**（FbxPose / FbxCluster）の変換は未実装（現時点では非対応）
- Skew（斜行）を含む Transform は正しく変換できない場合があります

## プロジェクト構成

```
FbxAxisUnitConverter/
├── FbxAxisUnitConverter.sln
└── FbxAxisUnitConverter/
    ├── main.cpp               # エントリポイント
    ├── ConvertOptions.h       # データ構造（AxisVector, ConvertOptions）
    ├── ArgumentParser.h/cpp   # CLI 引数パース・バリデーション
    ├── FbxAxisUnitConverter.h/cpp  # メインコントローラ
    ├── GeometryProcessor.h/cpp     # メッシュ頂点・法線変換
    ├── TransformProcessor.h/cpp    # ノード Transform 変換
    └── AnimProcessor.h/cpp         # アニメーション変換（TODO）
```

## ライセンス

[MIT License](LICENSE)
