# C++11 : 空の波括弧 `{}` による初期化

## `int`, POD, ポインタ

 `0` になる。POD, ポインタも同様

```C++
// http://ideone.com/Ok45HG
#include <stdio.h>
int main() {
    int i {};
    printf("i=%d\n", i);
}
// i=0
```


```C++
// http://ideone.com/XpQEql
#include <stdio.h>
#include <type_traits>
int main() {
    struct S { int x, y, z; };
    S s {};
    printf("is_pod=%d\n", std::is_pod<S>::value);
    printf("s{}={%d,%d,%d}\n", s.x, s.y, s.z);
}
// is_pod=1
// s{}={0,0,0}
```

 - 8.5.4/3 [dcl.init.list]
 - 8.5/8 [dcl.init]
 - 8.5/6 [dcl.init]
 - 103)


## class

デフォルトコンストラクタが呼ばれる

```C++
// http://ideone.com/uRxnq6
#include <stdio.h>
int main() {
    class C {
    public:
        C() : x(42) {}
        int x;
    };
    C c {};
    printf("c.x=%d\n", c.x);
}
// c.x=42
```

 - 8.5.4/3 [dcl.init.list]
 - 8.5/8 [dcl.init]
 - 8.5/7 [dcl.init]


## 配列

全要素について、`{}` が呼ばれる

 - デフォルトコンストラクタがあるなら、呼び出される
 - 組み込み型、POD なら `0` になる

```C++
// http://ideone.com/kLqPkZ
#include <stdio.h>
int main() {
    int a[4] {};
    for(const auto& e : a) {
        printf("%d\n", e);
    }
}
// 0
// 0
// 0
// 0
```

```C++
// http://ideone.com/ux8X3D
#include <stdio.h>
int main() {
    class C {
    public:
        C() : x(42) {}
        int x;
    };
    C a[4] {};
    for(const auto& e : a) {
        printf("%d\n", e.x);
    }
}
// 42
// 42
// 42
// 42
```

## メモ

 - この`{}`による初期化は `std::map` 等の要素にも適用されるので、以下のように初期化せずにインクリメント等の操作をしても良い
   - 値が `int` ではなくクラスなら、クラスのデフォルトコンストラクタが呼び出された後、後置インクリメント演算子が呼び出される

```C++
std::map<std::string, int> words;
for(std::string w; file >> w; ) {
    words[w]++;
}
```

 - http://www.reddit.com/r/cpp_questions/comments/1o0qwc/counting_instances_of_duplicate_words_in_a_linked/
 - http://blogs.msdn.com/b/vcblog/archive/2013/01/18/jumping-into-c.aspx


## 参照
 - POD
   - [What are Aggregates and PODs and how/why are they special?](http://stackoverflow.com/questions/4178175/what-are-aggregates-and-pods-and-how-why-are-they-special/7189821#7189821)
   - [Is this struct POD in C++11?](http://stackoverflow.com/questions/7169658/is-this-struct-pod-in-c11)

