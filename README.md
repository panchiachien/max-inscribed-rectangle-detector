# max-inscribed-rectangle-detector
檢測圖像中白色區塊的最大內接矩形 - 使用 C++ 和 OpenCV


## 功能
讀取黑底、含白色區塊的灰階圖像 → 二值化 → 尋找輪廓 → 對每個輪廓以旋轉搜尋計算最大內接矩形 → 選擇面積最大者，輸出中心點、寬、高、旋轉角度（0–360 度），並將矩形繪製於原圖。支援圓形、多邊形等任意形狀。

## 編譯
需要 CMake 與 OpenCV（例如 `sudo apt install libopencv-dev`）。
```bash
mkdir build && cd build
cmake .. && make
```

## 使用
```bash
./max_inscribed_rect <input_image> [output_image=result.png] [angle_step=1.0] [threshold=127]
```
範例：
```bash
./max_inscribed_rect ../examples/rotated_rect.png result.png
# 中心點: (199.134, 199.5)
# 寬度: 299
# 高度: 119
# 旋轉角度: 29.9999 度
```
- 角度為寬度邊相對於影像 x 軸的方向（影像座標系，順時針為正）。
- `angle_step` 越小越精確但越慢（搜尋範圍 0–180 度，矩形具 180 度對稱）。
- `examples/make_samples.py` 可產生範例圖（需 opencv-python）。
