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
- cast は C++ の 4 つの cast をしよう。明示的に

##Implementation
- 基本操作: Copy, Trim
- 縮小拡大: NearestNeighbor, Bilinear, Bicubic
- 加工関連: FlipX, FlipY, Rotate90, InvNegaPosi, Mozaic

##To Do
- Nearest Neighbor Filter, Bilinear Filter, Bicubic Filter の最適化
- Bilinear, Bicubic の SIMD 命令化 (SSE3 まで)。
- 明るさ調節、ぼかしの実装

##Problem
- (解決済み) アプリケーションが修了しない不具合。 (shared_ptr 化の影響？) Thanks to godai_0519
- (解決済み) Bilinear Filter は正常に作動しません。 (どこがおかしいのだろう…?)
- (解決済み) Multimedia Timer のせいか x86-64 built では動作しません。
