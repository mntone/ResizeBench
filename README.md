#ResizeBench

リサイズに関するベンチをとるとともに、高速化を実現していきます。

**Rgb24Image.cpp の評価をお願いします。**
  コードの見易さ、速さを求めています。
  ここをこうすればいいなどの意見がありましたら、是非とも Twitter @mntone に一声かけていただければ幸いです。

なお、Apache License, Version 2.0 を適応します。自由に使ってください。

ちなみにこのコードに関してですが、VS 2010 で開発動作することを念頭にしています。今後イントリンシック命令 SIMD を利用したプログラムも書いていく予定となっております。

##Rule
- タブをしよう。タブはスペース 4 つ分相当。
- スマポ使えるところは使っていこう。
- x86, x64 両方動作するようなコードに。

##To Do
- Nearest Neighbor Filter, Bilinear Filter の最適化
- Bilinear の SIMD 命令化。(over SSE3/over AVX) → AVX は整数演算? (注) SSE3＝Pen4。SSE3 以上で実装すれば基本動作する。
- 上下、左右、90°回転の実装
- 色反転、明るさ調節、ぼかし、モザイクの実装

##Problem
- (解決済み) アプリケーションが修了しない不具合。 (shared_ptr 化の影響？) Thanks to godai_0519
- (解決済み) Bilinear Filter は正常に作動しません。 (どこがおかしいのだろう…?)
- (解決済み) Multimedia Timer のせいか x86-64 built では動作しません。
