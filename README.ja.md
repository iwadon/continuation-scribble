# continuation-scribble

[English](README.md) | **日本語**

低レベルの継続(continuation)および協調的ファイバー(コルーチン)の実現可能性を検証するための実験プロジェクトです。C言語とアセンブリで複数のCPUアーキテクチャをサポートしています。

## サポートアーキテクチャ

- **arm64**: Apple Silicon (macOS)
- **x86_64**: Intel/AMD (macOS)
- **m68k**: Motorola 68000 (X68000、クロスコンパイル)

## ビルド方法

各アーキテクチャごとにninjaビルド設定があります：

```bash
ninja -C arm64      # ARM64 (macOS Apple Silicon)
ninja -C x86_64     # x86_64 (macOS Intel)
ninja -C m68k-xelf  # m68k (X68000 クロスコンパイル)
```

## テストの実行

### arm64 (Apple Silicon上でネイティブ実行)

```bash
./build/arm64-apple/arm64_ctx_test
./build/arm64-apple/fiber_test
./build/arm64-apple/cont_test
./build/arm64-apple/cont_clone_test
```

### x86_64 (ネイティブまたはRosetta 2)

```bash
./build/x86_64-apple/x86_64_ctx_test
./build/x86_64-apple/fiber_test
./build/x86_64-apple/cont_test
./build/x86_64-apple/cont_clone_test
```

### m68k (run68エミュレータが必要)

```bash
run68 ./build/m68k-xelf/m68k_ctx_test.x
run68 ./build/m68k-xelf/fiber_test.x
run68 ./build/m68k-xelf/cont_test.x
run68 ./build/m68k-xelf/cont_clone_test.x
```

## レイヤー構造

### 1. コンテキスト層 (最下層)

アーキテクチャ固有のレジスタ保存・復元。setjmp/longjmpに似たセマンティクス。

- `*_ctx_save()`: 保存時は0を返し、再開時は0以外を返す
- `*_ctx_resume()`: コンテキストを復元（戻らない）

### 2. ファイバー層

明示的なyieldによる協調的ユーザースペーススレッド。各ファイバーは独自のスタックを持つ。

### 3. 継続層

スタックイメージのキャプチャ・復元による完全な継続サポート。マルチショット継続（同じ継続を複数回再開可能）をサポート。

## 継続層の使い方

### 基本API

```c
#include "cont.h"

cont_t cont;
cont_init(&cont);           // 継続構造体を初期化

if (cont_save(&cont) == 0) {
    // 初回: 継続が保存された
    // ... 何かの処理 ...
    cont_resume(&cont);     // cont_saveの地点に戻る
}
// ここに再開される (cont_saveは1を返す)

cont_term(&cont);           // クリーンアップ
```

### マルチショット継続

```c
cont_t base, c1, c2;
cont_init(&base);
cont_init(&c1);
cont_init(&c2);

if (cont_save(&base) == 0) {
    cont_clone(&c1, &base);  // 継続をコピー
    cont_clone(&c2, &base);  // 複数回クローン可能
    cont_resume(&c1);        // 最初のコピーを再開
}
// c2は後でマルチショット動作のために再開可能
```

### 重要な使用上の注意

#### 1. 継続はキャプチャされるスタックの外に配置する

継続構造体はstatic/グローバルメモリまたはヒープに配置する必要があります。キャプチャされるスタック上に置いてはいけません。そうしないと、再開時に継続構造体自体が破壊されます。

```c
// 間違い: contがキャプチャされるスタック上にある
void bad_example(void) {
    cont_t cont;  // スタック変数 - 破壊される！
    cont_init(&cont);
    if (cont_save(&cont) == 0) {
        cont_resume(&cont);  // 未定義動作
    }
}

// 正しい: contがstaticメモリにある
static cont_t cont;
void good_example(void) {
    cont_init(&cont);
    if (cont_save(&cont) == 0) {
        cont_resume(&cont);  // 正しく動作する
    }
}
```

#### 2. cont_saveの前にcont_stack_baseを設定する

グローバル変数`cont_stack_base`は、キャプチャするスタック領域の上端をマークするために設定する必要があります。継続を使用する関数を呼び出す前に、エントリーポイントで設定してください。

```c
char *cont_stack_base;

static void get_stack_base(void) {
#if defined(__APPLE__) && defined(__aarch64__)
    asm volatile("mov %0, sp" : "=r"(cont_stack_base));
#elif defined(__human68k__)
    asm volatile("move.l %%a7, %0" : "=r"(cont_stack_base));
#elif defined(__x86_64__)
    asm volatile("movq %%rsp, %0" : "=r"(cont_stack_base));
#endif
}

int main(void) {
    get_stack_base();  // 最初にスタックベースを設定
    run_application(); // その後コードを呼び出す
    return 0;
}
```

#### 3. 適切なスタックキャプチャのためにネストした関数を使う

キャプチャされるスタック領域は、`cont_stack_base`から`cont_save`時点のSPまでです。意味のある状態をキャプチャするには、`cont_stack_base`の設定と`cont_save`の呼び出しの間に関数呼び出しがあることを確認してください。

```c
static cont_t cont;

static void do_work(void) {
    // この関数のスタックフレームがキャプチャされる
    if (cont_save(&cont) == 0) {
        // ...
        cont_resume(&cont);
    }
}

int main(void) {
    get_stack_base();
    do_work();  // do_workのスタックフレームがキャプチャされる
    return 0;
}
```

### エラーハンドリング

エラーを処理するには`cont_panic_fn`を設定します：

```c
void my_panic(const char *msg) {
    fprintf(stderr, "継続エラー: %s\n", msg);
}

int main(void) {
    cont_panic_fn = my_panic;
    // ...
}
```

## ライセンス

このプロジェクトはMIT No Attribution (MIT-0)ライセンスの下で公開されています。詳細は[LICENSE](LICENSE)を参照してください。
