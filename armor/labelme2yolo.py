import json
from pathlib import Path

# 数据集根目录（按你的实际路径改）
root = Path("/media/gfdgd_xi/新加卷1/BaiduNetdiskDownload/armor-database/armor_dataset")

# 类别映射，必须和 data.yaml 的 names 顺序一致
class_map = {
    "armor_red": 0,
    "armor_blue": 1,
}

def clamp(v, lo=0.0, hi=1.0):
    return max(lo, min(hi, v))

for split in ["train", "val", "test"]:
    label_dir = root / "labels" / split
    if not label_dir.exists():
        print(f"[skip] {label_dir} 不存在")
        continue

    json_files = list(label_dir.glob("*.json"))
    print(f"[{split}] 找到 {len(json_files)} 个 json")

    for json_file in json_files:
        with open(json_file, "r", encoding="utf-8") as f:
            data = json.load(f)

        w = data.get("imageWidth")
        h = data.get("imageHeight")
        if not w or not h:
            print(f"[warn] {json_file.name} 缺少 imageWidth/imageHeight，跳过")
            continue

        lines = []
        for shape in data.get("shapes", []):
            label = shape.get("label")
            if label not in class_map:
                print(f"[warn] {json_file.name} 未知类别 {label}，跳过")
                continue

            pts = shape.get("points", [])
            if not pts:
                continue

            stype = shape.get("shape_type", "rectangle")
            if stype == "rectangle" and len(pts) >= 2:
                (x1, y1), (x2, y2) = pts[0], pts[1]
            elif stype == "polygon" and len(pts) >= 3:
                xs = [p[0] for p in pts]
                ys = [p[1] for p in pts]
                x1, x2 = min(xs), max(xs)
                y1, y2 = min(ys), max(ys)
            else:
                continue

            xc = clamp((x1 + x2) / 2 / w)
            yc = clamp((y1 + y2) / 2 / h)
            bw = clamp(abs(x2 - x1) / w)
            bh = clamp(abs(y2 - y1) / h)

            if bw <= 0 or bh <= 0:
                continue

            lines.append(f"{class_map[label]} {xc:.6f} {yc:.6f} {bw:.6f} {bh:.6f}")

        # 用 imagePath 的 stem 作为 txt 文件名，保证和图片同名
        stem = Path(data.get("imagePath", json_file.stem)).stem
        txt_file = label_dir / f"{stem}.txt"
        with open(txt_file, "w", encoding="utf-8") as f:
            f.write("\n".join(lines))

    print(f"[{split}] 转换完成")
