import os
from PIL import Image, ImageSequence

def crop_center(img):
    width, height = img.size
    new_edge = min(width, height)
    left = (width - new_edge) // 2
    top = (height - new_edge) // 2
    right = left + new_edge
    bottom = top + new_edge
    return img.crop((left, top, right, bottom))

def process_image(file_path, output_dir):
    try:
        img = Image.open(file_path)
        output_path = os.path.join(output_dir, os.path.basename(file_path))
        if getattr(img, "is_animated", False):
            frames = []
            durations = []
            # Process each frame
            for frame in ImageSequence.Iterator(img):
                # Convert to RGBA for transparent channel support
                frame = frame.convert("RGBA")
                cropped_frame = crop_center(frame)
                resized_frame = cropped_frame.resize((500, 500), Image.LANCZOS)
                frames.append(resized_frame)
                durations.append(frame.info.get("duration", 100))
            frames[0].save(output_path,
                           save_all=True,
                           append_images=frames[1:],
                           loop=img.info.get("loop", 0),
                           duration=durations,
                           disposal=2)
        else:
            cropped = crop_center(img)
            resized = cropped.resize((500, 500), Image.LANCZOS)
            resized.save(output_path)
        print(f"Processed: {file_path}")
    except Exception as e:
        print(f"Error processing {file_path}: {e}")

def main(directory):
    output_dir = os.path.join(directory, "cropped")
    os.makedirs(output_dir, exist_ok=True)
    for file_name in os.listdir(directory):
        if file_name.lower().endswith(('.jpg', '.jpeg', '.png', '.gif')):
            file_path = os.path.join(directory, file_name)
            process_image(file_path, output_dir)

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="将指定目录下的图像从中心裁剪成 1:1 并调整尺寸至500*500")
    parser.add_argument("directory", type=str, help="图像所在的目录路径")
    args = parser.parse_args()
    main(args.directory)