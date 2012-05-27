#ResizeBench

リサイズに関するベンチをとるとともに、高速化を実現していきます。

**Rgb24Image.cpp の評価をお願いします。**
  コードの見易さ、速さを求めています。
  ここをこうすればいいなどの意見がありましたら、是非とも Twitter @mntone に一声かけていただければ幸いです。

なお、MIT License を適応します。自由に使ってください。(Apache License, Version 2.0 から変更しました)

、VS 2010 で開発動作することを念頭にしています。


##Rule
- タブをしよう。タブはスペース 4 つ分相当。
- スマポ使えるところは使っていこう。
- x86, x64 両方動作するようなコードに。
- cast は C++ の 4 つの cast をしよう。明示的に

##Implementation
- 基本操作: Copy, Trim
- 縮小拡大: NearestNeighbor, Bilinear, Bicubic
- 加工関連: FlipX, FlipY, Rotate90, InvNegaPosi, Mozaic, Blur

##To Do
- Nearest Neighbor Filter, Bilinear Filter, Bicubic Filter の最適化
- Bilinear, Bicubic の SIMD 命令化 (SSE3 まで)。
- 明るさ調節

##Problem
- (解決済み) アプリケーションが修了しない不具合。 (shared_ptr 化の影響？) Thanks to godai_0519
- (解決済み) Bilinear Filter は正常に作動しません。 (どこがおかしいのだろう…?)
- (解決済み) Multimedia Timer のせいか x86-64 built では動作しません。


以下、英語。

#ResizeBench

This program take the benchmark for the resize and try to realize speedy program.

*Please evaluate Rgb24Image.cpp.*
  I request easy to read and fast program.
  If you have opinion(s), you contact me (Twitter @mntone).

Also, I adapt this program to MIT License. Use this freely.

Use Visual Studio 2010.


##Rule
- Use tabs. A tab is 4 spaces.
- Try to use the smart pointer.
- Work on environmental both x86 and x64.
- Use the cast for C++.

##Implementation
- Basis Operation: Copy, Trim
- Shrink and Expand: NearestNeighbor, Bilinear, Bicubic
- Processing: FlipX, FlipY, Rotate90, InvNegaPosi, Mozaic, Blur

##To Do
- Optimize Nearest Neighbor Filter, Bilinear Filter, Bicubic Filter
- Rewrite Bilinear Filter, Bicubic Filter with SIMD (below SSE3)
- Adjust brightness
