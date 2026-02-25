# FbxAxisUnitConverter - 設計書

## 1. 概要 (Overview)

本ツールは、任意の軸系・単位で作成されたFBXファイルを、指定した軸系・単位のFBXファイルへ変換するC++ CLIツールである。
変換はバイナリレベルでの座標値の書き換え（Swizzle）を行い、追加ノードや変換ノードを挿入することなく、DCCツール上の見た目とTransform値の整合性を保つ。

### ターゲット環境

* **言語:** C++ 17以上
* **SDK:** Autodesk FBX SDK 2020.x 以降
* **OS:** Windows / macOS / Linux

### 主な機能

1. **軸変換:** FBX内部データの軸系から、指定した Up/Forward ベクトルの軸系へ変換。
2. **単位変換:** FBX SDK が定義する単位（`mm`, `cm`, `dm`, `m`, `km`, `inch`, `foot`, `yard`, `mile` 等）へのスケール変換。
3. **ジオメトリ焼き込み:** メッシュ頂点、法線、Binormal/Tangentの座標変換。
4. **階層構造の維持:** 親子関係を変更せず、各ノードのTransform値を新しい座標系に合わせて置換。
5. **共有メッシュ対応:** インスタンス化されたメッシュを検出し、重複変換を防止。

---

## 2. コマンドライン引数 (CLI Arguments)

### 2.1 引数一覧

| 引数 | 必須 | 説明 | 例 |
| --- | --- | --- | --- |
| `-i`, `--input` | Yes | 入力FBXファイルパス | `input.fbx` |
| `-o`, `--output` | Yes | 出力FBXファイルパス | `output.fbx` |
| `--up` | No | 出力のUp Vector（省略時: FBX内部データから取得） | `Y`, `Z`, `-Y`, `-Z` |
| `--forward` | No | 出力のForward Vector（省略時: FBX内部データから取得） | `X`, `Z`, `-X`, `-Z` |
| `--unit` | No | 出力の単位（省略時: FBX内部データから取得） | `cm`, `m`, `mm`, `inch`, `foot` |
| `--src-up` | No | 入力のUp Vectorを上書き（デフォルト: FBX内部データ） | `Z` |
| `--src-forward` | No | 入力のForward Vectorを上書き（デフォルト: FBX内部データ） | `-Y` |
| `--src-unit` | No | 入力の単位を上書き（デフォルト: FBX内部データ） | `cm` |

### 2.2 使用例

```bash
# FBX内部データから入力軸・単位を自動検出し、Y-Up / m に変換
FbxAxisUnitConverter -i input.fbx -o output.fbx --up Y --forward -Z --unit m

# 入力軸をZ-Up / Y-Forward として強制解釈し変換
FbxAxisUnitConverter -i input.fbx -o output.fbx --up Y --forward -Z --unit m --src-up Z --src-forward -Y

# 単位だけ変換（軸変換なし）
FbxAxisUnitConverter -i input.fbx -o output.fbx --unit m
```

### 2.3 Up/Forward Vector の受付値

以下の文字列（大文字・小文字不問）を受け付ける。

| 入力文字列 | 意味 |
| --- | --- |
| `X` | +X軸 |
| `-X` | −X軸 |
| `Y` | +Y軸 |
| `-Y` | −Y軸 |
| `Z` | +Z軸 |
| `-Z` | −Z軸 |

**バリデーション:** Up Vector と Forward Vector が平行（内積の絶対値が 1.0 以上）の場合は即座にエラー終了する。

```
Error: --up and --forward vectors must be orthogonal. Got up=Y, forward=Y.
```

### 2.4 単位 (Unit) の受付値

FBX SDK の `FbxSystemUnit` が提供する定義済み定数に対応する。

| 入力文字列 | FbxSystemUnit 定数 | 実スケール (mm基準) |
| --- | --- | --- |
| `mm` | `FbxSystemUnit::mm` | 1.0 |
| `cm` | `FbxSystemUnit::cm` | 10.0 |
| `dm` | `FbxSystemUnit::dm` | 100.0 |
| `m` | `FbxSystemUnit::m` | 1000.0 |
| `km` | `FbxSystemUnit::km` | 1,000,000.0 |
| `inch` | `FbxSystemUnit::Inch` | 25.4 |
| `foot` | `FbxSystemUnit::Foot` | 304.8 |
| `yard` | `FbxSystemUnit::Yard` | 914.4 |
| `mile` | `FbxSystemUnit::Mile` | 1,609,344.0 |

---

## 3. 入力データの自動検出

### 3.1 軸系の検出

入力FBX の `FbxGlobalSettings` から以下を取得する。

```cpp
FbxAxisSystem srcAxis = scene->GetGlobalSettings().GetAxisSystem();
```

`FbxAxisSystem` から Up/Front ベクトルの方向を逆算して変換行列を構築する。
`--src-up` / `--src-forward` が指定された場合はこの値を上書きする。

### 3.2 単位の検出

```cpp
FbxSystemUnit srcUnit = scene->GetGlobalSettings().GetSystemUnit();
```

`--src-unit` が指定された場合はこの値を上書きする。

---

## 4. アーキテクチャ (Architecture)

### 4.1 データフロー

1. **Parse Arguments:** Up/Forward/Unit 引数の解析とバリデーション（直交チェック）。
2. **Initialize:** FBX SDK Manager & Importer 初期化。
3. **Load:** FBXファイルをシーン（`FbxScene`）としてメモリに展開。
4. **Detect Source:** FBX内部データから軸系・単位を取得（引数による上書きを適用）。
5. **Build Transform Matrix:** ソース軸系→ターゲット軸系の変換行列 $M_{conv}$ とスケール係数 $S$ を計算。
6. **Pre-Process:** GlobalSettings の書き換え（ターゲット軸系・単位をセット）。
7. **Traverse & Convert:**
   * **Geometry Pass:** 全メッシュデータの頂点・法線を変換（重複排除）。
   * **Node Pass:** 再帰的にノードツリーを巡回し、LclTranslation, LclRotation, LclScaling を変換。
   * **Animation Pass:** アニメーションカーブの値を変換・チャンネル入れ替え。
8. **Export:** 変換後のシーンを新しいFBXファイルとして書き出し。

### 4.2 変換行列の構築ロジック

Up/Forward Vector からRight Vectorを外積で求め、$3 \times 3$ の座標変換行列を構築する。

```
Right = Forward × Up   (外積)

M_conv = [Right_target, Up_target, Forward_target] の列ベクトルで構成した行列
       × [Right_src,    Up_src,    Forward_src   ]^{-1}
```

スケール係数:
```
S = srcUnit.GetScaleFactor() / dstUnit.GetScaleFactor()
```

変換行列の行列式が $-1$ の場合（掌性が反転する場合）、ポリゴンの巻き順（Winding Order）を反転させる。

---

## 5. クラス設計 (Class Design)

### 5.1 `ConvertOptions` (Data Class)

変換パラメータをまとめて保持する。

```cpp
struct AxisVector {
    int axis;   // 0=X, 1=Y, 2=Z
    int sign;   // +1 or -1
};

struct ConvertOptions {
    std::string inputPath;
    std::string outputPath;

    // 出力軸系（必須）
    AxisVector dstUp;
    AxisVector dstForward;

    // 入力軸系（省略時はFBX内部データ）
    std::optional<AxisVector> srcUp;
    std::optional<AxisVector> srcForward;

    // 単位（省略時はFBX内部データ）
    std::optional<FbxSystemUnit> srcUnit;
    std::optional<FbxSystemUnit> dstUnit;
};
```

### 5.2 `ArgumentParser` (Helper)

* CLI引数を解析し `ConvertOptions` を構築する。
* Up/Forward の直交バリデーションを行い、非直交時はエラーメッセージを出力して終了。

### 5.3 `FbxAxisUnitConverter` (Main Controller)

* **役割:** SDK管理、全体の進行制御。
* **主なメソッド:**
  * `Run(ConvertOptions)` : メイン処理エントリ。
  * `BuildConversionMatrix()` : 変換行列・スケール係数の算出。
  * `ApplyGlobalSettings()` : FbxGlobalSettings への書き込み。

### 5.4 `GeometryProcessor` (Static Helper)

* **役割:** メッシュデータの書き換え。
* **主なメソッド:**
  * `ProcessMesh(FbxMesh*, FbxMatrix convMat, double scale)`
    * ControlPoints（頂点位置）の変換。
    * LayerElementNormal / Binormal / Tangent の変換（スケールなし）。
    * 掌性反転時の `FlipWindingOrder`。
  * `FlipWindingOrder(FbxMesh*)`

### 5.5 `TransformProcessor` (Static Helper)

* **役割:** ノードのLocal Transformを書き換える。
* **主なメソッド:**
  * `ProcessNode(FbxNode*, FbxMatrix convMat, double scale)`
    * `LclTranslation`: 変換行列を適用し、スケール係数を乗算。
    * `LclRotation`: Euler → Quaternion → 変換行列を適用 → Euler に戻す。
    * `LclScaling`: 変換行列の軸入れ替えに従って成分を並び替え。

### 5.6 `AnimProcessor` (Static Helper)

* **役割:** アニメーションカーブの変換。
* **主なメソッド:**
  * `ProcessAnimation(FbxScene*, FbxMatrix convMat, double scale)`
    * FbxAnimStack → FbxAnimLayer を巡回。
    * Translation カーブ: 変換行列に従ってXYZ成分を入れ替え・スケール乗算。
    * Rotation カーブ: 変換行列に従ってXYZ成分を入れ替え。

---

## 6. 詳細アルゴリズム・実装ガイド

### 6.1 メッシュ処理と重複防止

```cpp
void GeometryProcessor::ProcessMesh(FbxMesh* mesh, FbxMatrix convMat, double scale) {
    if (processedMeshes.count(mesh)) return; // 処理済みならスキップ

    // 頂点座標変換
    int numVerts = mesh->GetControlPointsCount();
    FbxVector4* verts = mesh->GetControlPoints();
    for (int i = 0; i < numVerts; ++i) {
        FbxVector4 transformed = convMat.MultNormalize(verts[i]);
        verts[i].Set(transformed[0] * scale, transformed[1] * scale, transformed[2] * scale, 1.0);
    }

    // 法線・Binormal・Tangent（スケールなし、方向のみ変換）
    // ...

    // 掌性反転時のWinding Order反転
    if (isHandednessFlipped) FlipWindingOrder(mesh);

    processedMeshes.insert(mesh);
}
```

### 6.2 グローバル設定の更新

```cpp
FbxAxisSystem dstAxisSystem(dstUp, dstForward); // FbxAxisSystem コンストラクタ
FbxGlobalSettings& settings = scene->GetGlobalSettings();
settings.SetAxisSystem(dstAxisSystem);
settings.SetSystemUnit(dstUnit);
```

### 6.3 バインドポーズ・スキンウェイト

キャラクターモデル（Skinned Mesh）の場合、以下も変換が必要。

* `FbxPose` (BindPose) に格納されているMatrixも同様に変換行列を適用。
* `FbxCluster` (Bone) の `TransformLinkMatrix` も変換。
* これを怠るとスキンメッシュが崩壊する。

---

## 7. 実装上の注意点と制限事項

### 制限事項

* **非直交行列:** Skew（斜行）が含まれるTransformは正しく変換できない可能性がある（一般的なゲームアセットでは稀）。
* **負のスケール:** 既に親階層で負のスケール（ミラーリング）が行われているデータに対してWinding Order反転を行うと、二重反転になる場合がある。`LclScaling` の符号を確認し、XOR的なロジックが必要。

### 検証方法

1. **Unity/Unrealでのインポート:**
   * "Bake Axis Conversion" オプションを **OFF** にしてインポートし、モデルが正立しているか確認。
   * TransformのScaleが `(1, 1, 1)` であるか確認。
   * Rotationが `(0, 0, 0)` （または意図した角度）であるか確認。

2. **アニメーション再生:**
   * ボーン構造が破綻していないか確認。
